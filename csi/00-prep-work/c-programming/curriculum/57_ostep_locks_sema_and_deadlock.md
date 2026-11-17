# Exercise 57: Concurrency — Ticket Locks, Semaphores, Barriers, RW Locks, and Lock Ordering

Bridges C11/C23 `<stdatomic.h>` and POSIX synchronization primitives directly to OSTEP Part II (Chapters 28–32) and `ostep-homework/threads-{locks,cv,sema,bugs}`.

---

# Ticket Spinlocks and Memory Ordering (OSTEP Ch. 28)

- `atomic_fetch_add_explicit(&lk->ticket, 1U, memory_order_relaxed)` assigns every arriving thread a unique ticket number without races.
- Polling `turn` with `memory_order_acquire` and releasing with `memory_order_release` creates a happens-before edge so shared state written inside the critical section is visible to the next lock holder.

---

# Semaphores, Reusable Barriers, and Writer-Preference RW Locks (OSTEP Ch. 31)

## Why Barriers Need a Generation Counter
```
Round 0: Thread 0,1 wait on my_gen=0 -> Thread 2 arrives (count==3):
           count=0, generation=1, broadcast -> wakes 0,1
         Fast Thread 2 loops to Round 1 BEFORE Thread 0 wakes:
           Thread 2 sees my_gen=1 != Thread 0's my_gen=0 (no wakeup corruption!)
```

## Preventing Writer Starvation
- Tracking `waiting_writers` and blocking new readers while `waiting_writers > 0` guarantees pending writers eventually acquire exclusive access instead of waiting forever behind overlapping readers.

---

# Deadlock Elimination via Total Lock Ordering (OSTEP Ch. 32)

- Coffman's circular-wait condition requires a cycle in the lock acquisition graph (`T1: A -> B`, `T2: B -> A`).
- Enforcing a strict total order by pointer address (`(uintptr_t)from < (uintptr_t)to`) makes cycles mathematically impossible, while checking `from == to` prevents self-deadlock.

---

# Active Recall Questions
- Why does a ticket lock prevent starvation better than a plain test-and-set spinlock?
- What race condition occurs if a reusable barrier resets `count = 0` and uses `while (count < threshold)` without a `generation` counter?
- Why does `account_transfer_ordered` compare `(uintptr_t)from < (uintptr_t)to` instead of checking account balances?
