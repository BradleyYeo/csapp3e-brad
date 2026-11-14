# Exercise 57: Robust Short-Count I/O and POSIX Threads Bounded Queue

Bridges POSIX file descriptors and concurrency to CS:APP Chapters 10 and 12 and `proxylab`: handling partial reads/writes and `EINTR` loops, plus mutex and condition-variable synchronization.

---

# Short Counts and Robust I/O (`rio_readn` / `rio_writen`)

## Why Single `read`/`write` Calls Fail Under Load
- On pipes, sockets, and terminals, `read(fd, buf, n)` may return fewer than `n` bytes simply because kernel buffers only hold part of the message so far.
- If a signal handler interrupts a blocked slow system call, it may return `-1` with `errno == EINTR`.
- `rio_readn_robust` and `rio_writen_robust` wrap `read`/`write` in loops that advance `usrbuf`, decrement `nleft`, retry on `EINTR`, and treat `read == 0` as genuine EOF.

---

# Concurrent Producer-Consumer Synchronization (`BoundedQueue`)

## Mesa Monitor Pattern (`pthread_mutex_t` + `pthread_cond_t`)
- Shared ring-buffer state (`buf`, `head`, `tail`, `count`) must only be inspected or modified while holding `q->mtx`.
- `pthread_cond_wait(&cond, &mtx)` atomically releases `mtx` and sleeps; upon waking, it re-acquires `mtx` before returning.

## Why `while` Loops Are Mandatory (Never `if`)
- Spurious wakeups or another competing consumer thread may steal the item between `pthread_cond_signal` and the waking thread reacquiring `mtx`.
- Re-checking `while (q->count == 0)` and `while (q->count == q->cap)` guarantees the invariant holds when execution proceeds.

---

# Active Recall Questions
- What distinguishes `read()` returning `0` from `read()` returning `-1` with `errno == EINTR`?
- Why can `rio_readn_robust` legitimately return fewer than `n` bytes requested, whereas `rio_writen_robust` never returns a partial success?
- What race condition occurs if `pthread_cond_wait` is guarded by `if (q->count == 0)` instead of `while (q->count == 0)` with two consumers?
