# Exercise 56: Signals, Coalesced Reaping, and ECF Race Conditions

Bridges POSIX process control to CS:APP Chapter 8 (Exceptional Control Flow) and `shlab`: unqueued pending signals, async-signal-safe handlers, `errno` preservation, and `sigprocmask` synchronization around `fork`.

---

# Signal Coalescing and Zombie Reaping

## Why `if (waitpid(...) > 0)` Leaks Zombies
- Standard POSIX signals (`SIGCHLD`) are recorded as a single pending bit per signal type, not a queue.
- If three child processes exit before the parent runs its `SIGCHLD` handler, only one `SIGCHLD` delivery may occur.
- A `SIGCHLD` handler must loop `while (waitpid(-1, nullptr, WNOHANG) > 0)` to drain every terminated child without blocking on still-running children.

---

# Async-Signal Safety and `errno` Preservation

## Rules Inside Signal Handlers (`CS:APP 8.5.5`)
- Call only async-signal-safe functions (`waitpid`, `_exit`, `write`); never call `printf`, `malloc`, or `free` inside a handler.
- Shared flags modified by handlers and read by normal code must have type `volatile sig_atomic_t`.
- Save `errno` on entry and restore it before returning so `waitpid` failing with `ECHILD` does not corrupt an unrelated `errno` in the interrupted function.

---

# Eliminating the `fork` / `SIGCHLD` Race Condition

```
Unsafe timeline:
  fork() --> child exits immediately --> SIGCHLD handler deletes job (NOT FOUND!)
         --> parent adds dead PID to JobTable (stale zombie/ghost entry!)

Safe timeline:
  sigprocmask(SIG_BLOCK, SIGCHLD)
    --> fork()
    --> child unblocks SIGCHLD and _exit(0)
    --> parent adds PID to JobTable (SIGCHLD still pending, blocked)
  sigprocmask(SIG_SETMASK, prev)  --> SIGCHLD delivered cleanly AFTER insert
```

---

# Active Recall Questions
- Why can calling `printf` or `malloc` inside a `SIGCHLD` handler deadlock a program?
- Why must a `SIGCHLD` handler save and restore `errno` even when it finishes with `waitpid` returning `0` or `-1`?
- Why must the child process restore `prev_mask` after `fork()` before calling `execve` or `_exit`?
