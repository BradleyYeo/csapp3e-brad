# Exercise 55: CPU Virtualization — Limited Direct Execution and Scheduling

Prepares core C timing, integer arithmetic, and scheduler state invariants for OSTEP Part I (Chapters 6–9).

---

# Limited Direct Execution (OSTEP Ch. 6)

## Why Syscall Timing Uses `CLOCK_MONOTONIC`
- User processes execute instructions directly on the CPU until a system call trap or timer interrupt transfers control to the kernel.
- `CLOCK_MONOTONIC` advances steadily without NTP wall-clock jumps, and subtracting two `struct timespec` values requires handling sub-second `tv_nsec` borrows.

---

# Scheduling Metrics and MLFQ (OSTEP Ch. 7–8)

## Turnaround vs Response Time
- **Turnaround time** ($T_{\text{completion}} - T_{\text{arrival}}$) rewards Shortest Job First (SJF) and Shortest Time-to-Completion First (STCF).
- **Response time** ($T_{\text{first\_run}} - T_{\text{arrival}}$) rewards Round-Robin time slicing.

## Anti-Gaming Allotment (Rule 4) and Priority Boost (Rule 5)
```
Without Rule 4:  [run 9ms] -> yield(I/O) -> reset budget -> stays top priority forever!
With Rule 4:     [run 3ms: used=3] -> [3ms: used=6] -> [3ms: used=9] -> [3ms: used=12 >= 10] -> DEMOTE!
```
- Accumulating `allotment_used` across yields closes the I/O-gaming loophole.
- `mlfq_priority_boost` lifts all jobs back to `num_levels - 1` periodically to cure starvation.

---

# Deterministic Stride Scheduling (OSTEP Ch. 9)

- Each job gets `stride = STRIDE_TICKET_SCALE / tickets` (inversely proportional to its share).
- At every quantum, the scheduler picks the runnable job with the lowest `pass` value and increments `pass += stride`.
- Over 60 quanta with tickets `100 : 50 : 150`, execution counts converge to exact `20 : 10 : 30` without randomness.

---

# Active Recall Questions
- Why can subtracting `end.tv_nsec - start.tv_nsec` overflow or underflow unsigned arithmetic if `end.tv_nsec < start.tv_nsec`?
- Why did original MLFQ fail against processes issuing tiny I/O calls just before their time slice expired?
- How does stride scheduling achieve exact proportional fairness deterministically compared to lottery scheduling?
