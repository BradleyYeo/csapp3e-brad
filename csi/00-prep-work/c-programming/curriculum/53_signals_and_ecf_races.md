# Exercise 53: Signals, Coalesced Reaping, and ECF Race Conditions (C23 + POSIX)

Teaches C23 and POSIX signal fundamentals from the ground up, then applies them to CS:APP Chapter 8 (Exceptional Control Flow), `shlab`, and [53_signals_and_ecf_races.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/csi/00-prep-work/c-programming/curriculum/53_signals_and_ecf_races.c).

---

# Fundamentals: ISO C23 Signals vs POSIX Extensions

## The C23 and POSIX Layer Boundary
- Strict `-std=c23` (`<signal.h>`, ISO C23 §7.14) defines only six signals (`SIGABRT`, `SIGFPE`, `SIGILL`, `SIGINT`, `SIGSEGV`, `SIGTERM`), `signal()`, `raise()`, `sig_atomic_t`, and lock-free `<stdatomic.h>` atomics.
- ISO C23 has no concept of child processes, signal masks, or process reaping.
- Defining `#define _POSIX_C_SOURCE 200809L` **before the first `#include`** exposes POSIX.1-2008 APIs under `-std=c23`: `SIGCHLD`, `sigaction()`, `sigprocmask()`, `sigset_t`, `fork()`, `waitpid()`, `_exit()`, `nanosleep()`, and POSIX's async-signal-safe function list.

```text
+-------------------------------------------------------------------+
| ISO C23 Core (-std=c23)                                           |
|   <signal.h> §7.14: 6 signals, signal(), raise(), sig_atomic_t    |
|   <stdatomic.h>: lock-free _Atomic types                          |
+-------------------------------------------------------------------+
| POSIX.1-2008 Extension (#define _POSIX_C_SOURCE 200809L)          |
|   SIGCHLD, sigaction(), sigprocmask(), sigset_t, fork(),          |
|   waitpid(), _exit(), nanosleep(), async-signal-safe syscalls     |
+-------------------------------------------------------------------+
| OS Kernel (Linux / WSL)                                           |
|   Per-process pending & blocked bitvectors, PCB zombie entries    |
+-------------------------------------------------------------------+
```

## C23 Language Idioms in This Exercise
- **`nullptr` (`nullptr_t`, C23 §6.4.1):** Type-safe null pointer constant that converts only to pointers or `bool`, never to `int`; used in `waitpid(-1, nullptr, WNOHANG)` and `sigprocmask(..., nullptr)`.
- **Native `bool`, `true`, `false` (C23 §6.4.1):** Keywords built into C23 without `<stdbool.h>`; the `#if __STDC_VERSION__ < 202311L` block only keeps older compilers working.
- **Empty-brace zero-initialisation `{}` (C23 §6.7.10):** Writing `struct sigaction sa = {};` or `JobTable jobs = {};` zeroes every member and padding byte in C23 (`{0}` stays valid for pre-C23 compatibility).
- **Unused parameters (C23 §6.7.12.4 / §6.9.2):** Pure C23 lets you write `static void handler([[maybe_unused]] int sig)` or omit the name (`static void handler(int)`), while `(void)sig;` works across both C23 and older C standards.
- **Strict conversions under `-Wconversion -Wsign-conversion`:** Compare `int total` against `size_t max_pids` safely by guarding `total >= 0` and casting `(size_t)total < max_pids`.

## What Exceptional Control Flow (ECF) Is
- Normal control flow executes instructions in sequence or jumps inside your own program.
- Exceptional control flow occurs when hardware or the kernel diverts execution to react to an external event (a syscall trap, a page fault, a timer interrupt, or a signal).
- A signal is a small numbered notification (such as `SIGCHLD`, `SIGINT`, or `SIGTERM`) that the kernel delivers to a process.

## Kernel Bitvectors: `pending` and `blocked`
- The kernel maintains two bitvectors for every process:
  - `pending`: bit `k` is `1` if signal `k` arrived and has not been delivered yet.
  - `blocked` (the signal mask): bit `k` is `1` if the process asked the kernel to delay delivery of signal `k`.
- Whenever the kernel switches back from kernel mode to user mode for a process, it computes `pending & ~blocked`.
- If any bit is `1`, the kernel clears that pending bit and invokes the registered handler before resuming normal code.

```text
Kernel state for Parent Process:
  pending: [ ... | SIGCHLD=1 | ... ]   (at most ONE bit per signal — NOT a counter!)
  blocked: [ ... | SIGCHLD=0 | ... ]   (controlled by sigprocmask)
                       |
                       v
  deliverable = pending & ~blocked
  If non-zero --> kernel clears pending bit and invokes handler on parent's stack
```

- Because `pending` holds only one bit per signal number, signals are **never queued or counted**.

## Child Termination, Zombies, and `_exit()` vs `_Exit()` vs `exit()`
- When a child terminates, the kernel frees its memory and file descriptors, but keeps its Process Control Block entry (PID and exit status) as a **zombie**.
- At the same moment, the kernel sets `SIGCHLD = 1` in the parent's `pending` bitvector; the zombie disappears only when the parent reaps it with `waitpid()`.
- Know the three termination functions across C23 and POSIX:
  - `exit()` (ISO C23): flushes inherited `stdio` buffers and runs `atexit` handlers, which duplicates buffered output if called in a forked child.
  - `_Exit()` (ISO C23 §7.24.4.4): terminates immediately without flushing `stdio` buffers or running `atexit` handlers; safe inside signal handlers.
  - `_exit()` (POSIX): terminates immediately in the kernel with the same no-flush guarantee and is also POSIX async-signal-safe.

## Installing Handlers with `sigaction()` (Why Not ISO C23 `signal()`)
- ISO C23 §7.14.1.1 leaves `signal()` behaviour implementation-defined on handler reset and signal masking; some systems reset the handler to `SIG_DFL` after a single signal arrives.
- POSIX `sigaction()` guarantees persistent handler registration, automatically blocks the caught signal while its handler runs, and supports `SA_RESTART` to restart interrupted slow syscalls.
- Build a `sigset_t` with `sigemptyset(&mask)` and `sigaddset(&mask, SIGCHLD)`, then update the kernel `blocked` bitvector with `sigprocmask(SIG_BLOCK, &mask, &prev)` and restore it with `sigprocmask(SIG_SETMASK, &prev, nullptr)`.

## How `waitpid(-1, &status, WNOHANG)` Behaves
- `pid = -1` matches **any** child process of the caller, and `WNOHANG` returns immediately without blocking on running children.
- Every call returns one of four outcomes:
  - `pid > 0`: the PID of one terminated child that the kernel just reaped.
  - `0`: live children exist, but none have terminated yet.
  - `-1` with `errno == ECHILD`: this process currently has no children at all.
  - `-1` with `errno == EINTR`: another signal interrupted `waitpid`; retry the loop.

## Shared State in Signal Handlers: C23 §7.14.1.1 Rules
- ISO C23 §7.14.1.1 states that accessing an object with static or thread storage duration from an asynchronous signal handler causes **undefined behaviour** unless that object is either:
  - `volatile sig_atomic_t`, OR
  - a **lock-free atomic object** (`_Atomic` from `<stdatomic.h>` where `atomic_is_lock_free` is true).
- `volatile` forces the CPU to load `g_reaped_count` from memory on each iteration so `-O2` cannot cache it in a register and hang `while (g_reaped_count < 4)`.
- `sig_atomic_t` ensures single-instruction reads and writes without torn values.
- Note why `g_reaped_count++` works here: `sigaction` blocks nested `SIGCHLD` delivery during the handler, and `main` only writes `g_reaped_count = 0` before spawning children.

---

# Concept 1: Signal Coalescing and Non-Blocking Reaping (Drill 1)

## Why `if (waitpid(...) > 0)` Leaks Zombies
- Suppose three children exit while `SIGCHLD` is blocked or while the `SIGCHLD` handler is already running.
- Child 1 sets the `SIGCHLD` pending bit to `1`; Child 2 and Child 3 see the bit is already `1` and drop their extra notifications.
- When the parent runs the handler, the kernel delivers **one** `SIGCHLD`, not three.

```text
Child 1 exits --> pending[SIGCHLD] = 1
Child 2 exits --> pending[SIGCHLD] is ALREADY 1 (coalesced!)
Child 3 exits --> pending[SIGCHLD] is ALREADY 1 (coalesced!)
                       |
                       v
Parent runs handler ONCE:
  BAD:  if (waitpid(-1, &st, WNOHANG) > 0)    --> reaps 1 child, LEAKS 2 zombies
  GOOD: while (waitpid(-1, &st, WNOHANG) > 0) --> drains all 3 zombies in one call
```

## The `WNOHANG` Drain Loop Invariant
- Always reap in a `while` loop until `waitpid(-1, &status, WNOHANG)` stops returning positive PIDs.
- In `reap_all_zombies_nonblocking`, keep looping and incrementing `total` even if `(size_t)total >= max_pids`; only store `reaped_pids[total] = pid` when `reaped_pids != nullptr && (size_t)total < max_pids`.
- If `waitpid` returns `0` or returns `-1` with `errno == ECHILD`, break and return `total`.
- If `waitpid` returns `-1` with `errno == EINTR`, continue the loop; for any other `errno`, return `-1`.

---

# Concept 2: Async-Signal Safety and `errno` Preservation (Drill 2)

## C23 vs POSIX Async-Signal-Safe Functions
- Pure ISO C23 §7.14.1.1 permits almost nothing inside an asynchronous handler except `abort()`, `_Exit()`, `quick_exit()`, `signal()` for the same signal, and lock-free `<stdatomic.h>` operations; calling any other standard library function (including `malloc` or `printf`) is undefined behaviour.
- POSIX extends that guarantee to an explicit async-signal-safe list (`waitpid`, `_exit`, `write`, `sigprocmask`), while `malloc`, `free`, and `printf` remain unsafe because they hold internal locks.
- A handler runs on the **exact same thread** it interrupted, so trying to acquire a lock already held by `main` causes a permanent deadlock.

```text
main() calls malloc()
  --> acquires heap_lock
  --> [SIGCHLD arrives right here!]
        --> handler calls printf() / malloc()
        --> tries to acquire heap_lock (already held by main!)
        --> PERMANENT DEADLOCK (main cannot resume until handler returns)
```

## Preserving `errno` Across Handler Boundaries
- Under ISO C23 §7.5, `errno` is a per-thread modifiable lvalue shared between normal code and any signal handler that interrupts that thread.
- In `sigchld_preserving_handler`, the final `waitpid(-1, nullptr, WNOHANG)` call after reaping the last child returns `-1` and sets `errno = ECHILD`.
- If the handler does not restore the old `errno`, interrupted code in `main` sees `ECHILD` instead of its own `errno` (such as `EINVAL` in Drill 2's test).
- Save `int saved_errno = errno;` on entry and restore `errno = saved_errno;` before returning; following John Ousterhout's information-hiding principle, this hides handler side effects from callers.

---

# Concept 3: Eliminating the `fork` / `SIGCHLD` Race Condition (Drill 3)

## The Order-Violation Race
- After `fork()`, the kernel scheduler makes **no guarantee** about whether the parent or the child runs first.
- If the child exits immediately before the parent inserts its PID into `JobTable`, the handler tries to delete a job that does not exist yet, and the parent later inserts a dead PID permanently.

```text
UNSAFE (race condition):
  parent: fork()
            |---> child runs immediately and calls _exit(0)
            |---> kernel delivers SIGCHLD --> handler deletes PID (NOT FOUND!)
  parent: jobs->pids[count++] = pid  (inserts a dead ghost job!)

SAFE (block SIGCHLD across fork + table update):
  parent: sigprocmask(SIG_BLOCK, &mask, &prev)    // block SIGCHLD
  parent: pid = fork()
            |---> child: sigprocmask(SIG_SETMASK, &prev, nullptr)
            |---> child: _exit(0)                 // sets parent pending[SIGCHLD]=1 (held!)
  parent: jobs->pids[jobs->count++] = pid         // table updated FIRST
  parent: sigprocmask(SIG_SETMASK, &prev, nullptr)// unblock -> SIGCHLD delivered cleanly NOW
```

## Why the Child Must Restore `prev`
- A child inherits its parent's `blocked` signal mask across both `fork()` and `execve()`.
- If the child skips `sigprocmask(SIG_SETMASK, &prev, nullptr)`, any program it executes starts with `SIGCHLD` permanently masked.

## Define Races Out of Existence (Never Sleep)
- Inserting `sleep()` in the child only hides the race on an idle machine and fails unpredictably under CPU load.
- Blocking `SIGCHLD` across `fork()` and `JobTable` registration defines the bad interleaving out of existence.

---

# Pitfalls to Watch For in `53_signals_and_ecf_races.c`

- **Leaked signal mask on `fork()` failure:** If `fork() < 0`, restore `sigprocmask(SIG_SETMASK, &prev, nullptr)` before returning `false`, or `SIGCHLD` stays blocked in the parent.
- **Null output pointer:** In `spawn_and_register_job_safe`, guard `if (out_child_pid != nullptr)` before writing `*out_child_pid = pid`.
- **Stopping early on `max_pids`:** In `reap_all_zombies_nonblocking`, keep reaping zombies even if `(size_t)total >= max_pids`, and stop storing into `reaped_pids` once full.
- **Wrong `sigprocmask` restoration flag:** Use `SIG_SETMASK` with `&prev` rather than `SIG_UNBLOCK` with `&mask` so callers that already blocked `SIGCHLD` keep their original mask intact.
- **Signed/unsigned mismatch (`-Wsign-conversion`):** Cast `(size_t)total < max_pids` only after confirming `total >= 0` so negative integers never wrap under `-Werror`.

---

# Active Recall Questions

- What belongs to ISO C23 `<signal.h>` (§7.14), and why does `53_signals_and_ecf_races.c` define `_POSIX_C_SOURCE 200809L` before the first `#include`?
- Under ISO C23 §7.14.1.1, which two types of static/thread-duration objects may an asynchronous signal handler safely access, and why is accessing anything else undefined behaviour?
- Why does `53_signals_and_ecf_races.c` use `nullptr` and `= {}` / `= {0}`, and how does C23's `nullptr` catch bugs that `NULL` misses?
- Contrast `exit()`, `_Exit()` (C23), and `_exit()` (POSIX) inside a forked child and inside a signal handler.
- Why does ISO C23 `signal()` create portability traps that POSIX `sigaction()` eliminates?
- What are the four return outcomes of `waitpid(-1, &status, WNOHANG)`, and which two stop the drain loop cleanly?
- Trace step by step how calling `malloc` or `printf` inside a `SIGCHLD` handler causes both C23 undefined behaviour and a single-threaded deadlock.
- Why must `sigchld_preserving_handler` save and restore `errno` even though it checks no errors itself?
- Why must you restore `prev` using `SIG_SETMASK` in both the parent (after `JobTable` insertion) and the child (before `_exit` or `execve`)?
