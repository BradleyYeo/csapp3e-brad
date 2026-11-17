/*
 * Exercise 57: Concurrency — Ticket Locks, Semaphores, Barriers, RW Locks & Lock Ordering (OSTEP Ch. 28–32)
 *
 * Compile and run:
 *   make -C curriculum 57_ostep_locks_sema_and_deadlock && ./curriculum/57_ostep_locks_sema_and_deadlock
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  atomic_uint ticket;
  atomic_uint turn;
} TicketLock;

typedef struct {
  int value;
  pthread_mutex_t mtx;
  pthread_cond_t cv;
} Zem;

typedef struct {
  uint32_t threshold;
  uint32_t count;
  uint32_t generation;
  pthread_mutex_t mtx;
  pthread_cond_t cv;
} ReusableBarrier;

typedef struct {
  uint32_t active_readers;
  uint32_t waiting_writers;
  bool writer_active;
  pthread_mutex_t mtx;
  pthread_cond_t can_read;
  pthread_cond_t can_write;
} NoStarveRWLock;

typedef struct {
  int64_t balance;
  pthread_mutex_t mtx;
} LockedAccount;

/*
 * Drill 1: Fair Ticket Spinlock with Acquire-Release Ordering (OSTEP Ch. 28)
 *
 * A simple Test-and-Set spinlock can starve waiting threads; a Ticket Lock
 * serves waiters in strict FIFO arrival order.
 *
 * Requirements:
 * - ticket_lock_init: initialize `ticket` and `turn` to 0 using `atomic_init`.
 * - ticket_lock_acquire:
 *     `unsigned int my_ticket = atomic_fetch_add_explicit(&lk->ticket, 1U, memory_order_relaxed);`
 *     Spin `while (atomic_load_explicit(&lk->turn, memory_order_acquire) != my_ticket)`
 *     calling `sched_yield()`.
 * - ticket_lock_release:
 *     `atomic_fetch_add_explicit(&lk->turn, 1U, memory_order_release);`
 */
static void ticket_lock_init(TicketLock *lk) {
  (void)lk;
  /* TODO: implement */
}

static void ticket_lock_acquire(TicketLock *lk) {
  (void)lk;
  /* TODO: implement */
}

static void ticket_lock_release(TicketLock *lk) {
  (void)lk;
  /* TODO: implement */
}

/*
 * Drill 2: Zemaphore — Portable Counting Semaphore (OSTEP Ch. 31)
 *
 * macOS deprecates unnamed `sem_init`, and OSTEP implements `Zem_t` directly
 * from a mutex + condition variable to reveal semaphore internals.
 *
 * Requirements:
 * - zem_init: return false if z == nullptr || init_value < 0.
 *   Set `z->value = init_value` and initialize `mtx` and `cv`.
 * - zem_destroy: destroy `cv` and `mtx`.
 * - zem_wait: lock `mtx`, wait `while (z->value <= 0)` on `cv`, decrement `z->value--`, unlock.
 * - zem_post: lock `mtx`, increment `z->value++`, signal `cv`, unlock.
 */
static bool zem_init(Zem *z, int init_value) {
  (void)z;
  (void)init_value;
  return false; /* TODO: implement */
}

static void zem_destroy(Zem *z) {
  (void)z;
  /* TODO: implement */
}

static void zem_wait(Zem *z) {
  (void)z;
  /* TODO: implement */
}

static void zem_post(Zem *z) {
  (void)z;
  /* TODO: implement */
}

/*
 * Drill 3: Reusable Generation-Based Thread Barrier (OSTEP `threads-sema/barrier.c`)
 *
 * A barrier without a `generation` counter breaks on consecutive rounds when fast
 * threads re-enter the barrier before slow threads wake from the previous round.
 *
 * Requirements:
 * - barrier_init: return false if b == nullptr || num_threads == 0.
 *   Initialize `threshold = num_threads`, `count = 0`, `generation = 0`, `mtx`, `cv`.
 * - barrier_destroy: destroy `cv` and `mtx`.
 * - barrier_wait:
 *     Lock `mtx`. Save `uint32_t my_gen = b->generation;`.
 *     Increment `b->count`.
 *     If `b->count == b->threshold`:
 *       Reset `b->count = 0`, increment `b->generation++`, broadcast `b->cv`,
 *       unlock `mtx`, and return true (this thread tripped the barrier).
 *     Else:
 *       Wait `while (b->generation == my_gen)` on `b->cv`, unlock `mtx`, and return false.
 */
static bool barrier_init(ReusableBarrier *b, uint32_t num_threads) {
  (void)b;
  (void)num_threads;
  return false; /* TODO: implement */
}

static void barrier_destroy(ReusableBarrier *b) {
  (void)b;
  /* TODO: implement */
}

static bool barrier_wait(ReusableBarrier *b) {
  (void)b;
  return false; /* TODO: implement */
}

/*
 * Drill 4: Starvation-Free (Writer-Preference) Reader-Writer Lock
 * (OSTEP Ch. 31 `reader-writer-nostarve.c`)
 *
 * Naive RW locks allow a continuous stream of readers to starve writers forever.
 * Blocking new readers whenever `waiting_writers > 0` prevents writer starvation.
 *
 * Requirements:
 * - rwlock_init / rwlock_destroy: init/destroy `mtx`, `can_read`, `can_write`;
 *   zero counters and `writer_active = false`.
 * - rwlock_acquire_read: wait `while (rw->writer_active || rw->waiting_writers > 0)` on `can_read`,
 *   then `rw->active_readers++`.
 * - rwlock_release_read: decrement `rw->active_readers`; if 0 and `waiting_writers > 0`,
 *   signal `can_write`.
 * - rwlock_acquire_write: increment `rw->waiting_writers++`, wait
 *   `while (rw->writer_active || rw->active_readers > 0)` on `can_write`,
 *   decrement `rw->waiting_writers--`, set `rw->writer_active = true`.
 * - rwlock_release_write: set `rw->writer_active = false`; if `rw->waiting_writers > 0`,
 *   signal `can_write`, else broadcast `can_read`.
 */
static bool rwlock_init(NoStarveRWLock *rw) {
  (void)rw;
  return false; /* TODO: implement */
}

static void rwlock_destroy(NoStarveRWLock *rw) {
  (void)rw;
  /* TODO: implement */
}

static void rwlock_acquire_read(NoStarveRWLock *rw) {
  (void)rw;
  /* TODO: implement */
}

static void rwlock_release_read(NoStarveRWLock *rw) {
  (void)rw;
  /* TODO: implement */
}

static void rwlock_acquire_write(NoStarveRWLock *rw) {
  (void)rw;
  /* TODO: implement */
}

static void rwlock_release_write(NoStarveRWLock *rw) {
  (void)rw;
  /* TODO: implement */
}

/*
 * Drill 5: Deadlock Avoidance via Total Lock Ordering (OSTEP Ch. 32 `vector-global-order.c`)
 *
 * Acquiring `from->mtx` then `to->mtx` deadlocks when Thread 1 calls transfer(A, B)
 * while Thread 2 calls transfer(B, A), or self-deadlocks on transfer(A, A).
 *
 * Requirements:
 * - Return false if from == nullptr || to == nullptr || amount < 0.
 * - If `from == to`: lock `from->mtx`, check `bool ok = (from->balance >= amount)`,
 *   unlock `from->mtx`, return `ok` (do NOT lock twice!).
 * - Otherwise: acquire the lock with lower pointer address `(uintptr_t)from < (uintptr_t)to`
 *   first, then the higher address lock second.
 * - While holding both locks: if `from->balance >= amount`, subtract from `from` and
 *   add to `to` (`ok = true`), else `ok = false`.
 * - Unlock both locks and return `ok`.
 */
static bool account_transfer_ordered(LockedAccount *from, LockedAccount *to,
                                     int64_t amount) {
  (void)from;
  (void)to;
  (void)amount;
  return false; /* TODO: implement */
}

/* ── Test Harnesses ───────────────────────────────────────────────────────── */

typedef struct {
  TicketLock *lk;
  int64_t *counter;
  int iters;
} TicketWorkerArgs;

static void *ticket_worker(void *arg) {
  TicketWorkerArgs *a = (TicketWorkerArgs *)arg;
  for (int i = 0; i < a->iters; i++) {
    ticket_lock_acquire(a->lk);
    (*a->counter)++;
    ticket_lock_release(a->lk);
  }
  return nullptr;
}

typedef struct {
  Zem *done_sem;
  int *result;
} ForkJoinArgs;

static void *fork_join_child(void *arg) {
  ForkJoinArgs *a = (ForkJoinArgs *)arg;
  *a->result = 42;
  zem_post(a->done_sem);
  return nullptr;
}

typedef struct {
  ReusableBarrier *bar;
  int *phase_hits;
  int leader_count;
} BarrierArgs;

static void *barrier_worker(void *arg) {
  BarrierArgs *a = (BarrierArgs *)arg;
  for (int p = 0; p < 3; p++) {
    __atomic_fetch_add(&a->phase_hits[p], 1, __ATOMIC_RELAXED);
    if (barrier_wait(a->bar)) {
      a->leader_count++;
    }
    assert(__atomic_load_n(&a->phase_hits[p], __ATOMIC_RELAXED) == 4);
  }
  return nullptr;
}

typedef struct {
  NoStarveRWLock *rw;
  int *shared_val;
  int *active_writers_in_cs;
  int loops;
} RwArgs;

static void *rw_reader(void *arg) {
  RwArgs *a = (RwArgs *)arg;
  int prev = 0;
  for (int i = 0; i < a->loops; i++) {
    rwlock_acquire_read(a->rw);
    assert(__atomic_load_n(a->active_writers_in_cs, __ATOMIC_RELAXED) == 0);
    int cur = *a->shared_val;
    assert(cur >= prev);
    prev = cur;
    rwlock_release_read(a->rw);
  }
  return nullptr;
}

static void *rw_writer(void *arg) {
  RwArgs *a = (RwArgs *)arg;
  for (int i = 0; i < a->loops; i++) {
    rwlock_acquire_write(a->rw);
    int w = __atomic_add_fetch(a->active_writers_in_cs, 1, __ATOMIC_RELAXED);
    assert(w == 1);
    (*a->shared_val)++;
    __atomic_sub_fetch(a->active_writers_in_cs, 1, __ATOMIC_RELAXED);
    rwlock_release_write(a->rw);
  }
  return nullptr;
}

typedef struct {
  LockedAccount *from;
  LockedAccount *to;
  int iters;
} TransferArgs;

static void *transfer_worker(void *arg) {
  TransferArgs *a = (TransferArgs *)arg;
  for (int i = 0; i < a->iters; i++) {
    account_transfer_ordered(a->from, a->to, 1);
  }
  return nullptr;
}

int main(void) {
  /* Drill 1: Ticket spinlock mutual exclusion */
  TicketLock tlock;
  ticket_lock_init(&tlock);
  int64_t tcounter = 0;
  pthread_t tthr[4];
  TicketWorkerArgs tw[4];
  for (size_t i = 0; i < 4; i++) {
    tw[i] = (TicketWorkerArgs){.lk = &tlock, .counter = &tcounter, .iters = 5000};
    assert(pthread_create(&tthr[i], nullptr, ticket_worker, &tw[i]) == 0);
  }
  for (size_t i = 0; i < 4; i++) {
    assert(pthread_join(tthr[i], nullptr) == 0);
  }
  assert(tcounter == 20000);

  /* Drill 2: Zemaphore fork/join signaling */
  Zem bad_sem;
  assert(zem_init(nullptr, 1) == false);
  assert(zem_init(&bad_sem, -1) == false);

  Zem done_sem;
  assert(zem_init(&done_sem, 0) == true);
  int child_val = 0;
  ForkJoinArgs fj = {.done_sem = &done_sem, .result = &child_val};
  pthread_t fj_thr;
  assert(pthread_create(&fj_thr, nullptr, fork_join_child, &fj) == 0);
  zem_wait(&done_sem);
  assert(child_val == 42);
  assert(pthread_join(fj_thr, nullptr) == 0);
  zem_destroy(&done_sem);

  /* Drill 3: Reusable multi-phase barrier */
  ReusableBarrier bar;
  assert(barrier_init(&bar, 0) == false);
  assert(barrier_init(&bar, 4) == true);
  int phase_hits[3] = {0, 0, 0};
  pthread_t bthr[4];
  BarrierArgs bargs[4];
  for (size_t i = 0; i < 4; i++) {
    bargs[i] = (BarrierArgs){.bar = &bar, .phase_hits = phase_hits, .leader_count = 0};
    assert(pthread_create(&bthr[i], nullptr, barrier_worker, &bargs[i]) == 0);
  }
  int total_leaders = 0;
  for (size_t i = 0; i < 4; i++) {
    assert(pthread_join(bthr[i], nullptr) == 0);
    total_leaders += bargs[i].leader_count;
  }
  assert(total_leaders == 3);
  barrier_destroy(&bar);

  /* Drill 4: Starvation-free reader-writer lock */
  NoStarveRWLock rw;
  assert(rwlock_init(&rw) == true);
  int shared_val = 0;
  int active_writers = 0;
  RwArgs rwa = {.rw = &rw, .shared_val = &shared_val, .active_writers_in_cs = &active_writers, .loops = 500};
  pthread_t rthr[2], wthr[2];
  for (size_t i = 0; i < 2; i++) {
    assert(pthread_create(&rthr[i], nullptr, rw_reader, &rwa) == 0);
    assert(pthread_create(&wthr[i], nullptr, rw_writer, &rwa) == 0);
  }
  for (size_t i = 0; i < 2; i++) {
    assert(pthread_join(rthr[i], nullptr) == 0);
    assert(pthread_join(wthr[i], nullptr) == 0);
  }
  assert(shared_val == 1000);
  rwlock_destroy(&rw);

  /* Drill 5: Total lock ordering prevents A<->B deadlock and A->A self-deadlock */
  LockedAccount acct_a = {.balance = 10000};
  LockedAccount acct_b = {.balance = 10000};
  assert(pthread_mutex_init(&acct_a.mtx, nullptr) == 0);
  assert(pthread_mutex_init(&acct_b.mtx, nullptr) == 0);

  assert(account_transfer_ordered(&acct_a, &acct_a, 500) == true);
  assert(account_transfer_ordered(&acct_a, &acct_a, 99999) == false);

  pthread_t xfer[2];
  TransferArgs xa = {.from = &acct_a, .to = &acct_b, .iters = 2000};
  TransferArgs xb = {.from = &acct_b, .to = &acct_a, .iters = 2000};
  assert(pthread_create(&xfer[0], nullptr, transfer_worker, &xa) == 0);
  assert(pthread_create(&xfer[1], nullptr, transfer_worker, &xb) == 0);
  assert(pthread_join(xfer[0], nullptr) == 0);
  assert(pthread_join(xfer[1], nullptr) == 0);
  assert(acct_a.balance + acct_b.balance == 20000);

  pthread_mutex_destroy(&acct_a.mtx);
  pthread_mutex_destroy(&acct_b.mtx);

  printf("[PASS] Exercise 57: OSTEP Locks, Semaphores, Barriers & Lock Ordering verified.\n");
  return EXIT_SUCCESS;
}
