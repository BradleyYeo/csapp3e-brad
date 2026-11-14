/*
 * Exercise 57: Robust Short-Count I/O & POSIX Threads Bounded Queue (CS:APP Ch. 10 & 12 / proxylab)
 *
 * Compile and run:
 *   make -C curriculum 57_robust_io_and_pthreads && ./curriculum/57_robust_io_and_pthreads
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  int *buf;
  size_t cap;
  size_t head;
  size_t tail;
  size_t count;
  pthread_mutex_t mtx;
  pthread_cond_t not_empty;
  pthread_cond_t not_full;
} BoundedQueue;

/*
 * Drill 1: CS:APP Robust Unbuffered Read (`rio_readn`)
 *
 * POSIX read(2) on pipes and TCP sockets can return fewer bytes than requested
 * ("short count") or fail with errno == EINTR if interrupted by a signal.
 *
 * Requirements:
 * - Return 0 immediately if n == 0.
 * - If usrbuf == nullptr, set errno = EINVAL and return -1.
 * - Loop until `nleft == 0` or EOF (`read` returns 0):
 *     - On `nread < 0`:
 *         If `errno == EINTR`, reset `nread = 0` and retry.
 *         Otherwise return -1.
 *     - On `nread == 0`: break (EOF reached).
 *     - Advance buffer pointer and subtract `nread` from `nleft`.
 * - Return total bytes read `(ssize_t)(n - nleft)`.
 */
static ssize_t rio_readn_robust(int fd, void *usrbuf, size_t n) {
  (void)fd;
  (void)usrbuf;
  (void)n;
  return -1; /* TODO: implement */
}

/*
 * Drill 2: CS:APP Robust Unbuffered Write (`rio_writen`)
 *
 * Requirements:
 * - Return 0 immediately if n == 0.
 * - If usrbuf == nullptr, set errno = EINVAL and return -1.
 * - Loop until `nleft == 0`:
 *     - On `nwritten <= 0`:
 *         If `nwritten < 0 && errno == EINTR`, reset `nwritten = 0` and retry.
 *         Otherwise return -1.
 *     - Advance buffer pointer and subtract `nwritten` from `nleft`.
 * - Return `(ssize_t)n`.
 */
static ssize_t rio_writen_robust(int fd, const void *usrbuf, size_t n) {
  (void)fd;
  (void)usrbuf;
  (void)n;
  return -1; /* TODO: implement */
}

/*
 * Drill 3: Thread-Safe Bounded Queue (CS:APP `sbuf` / Monitor Pattern)
 *
 * Requirements for bqueue_init:
 * - Return false if q == nullptr || cap == 0 || cap > SIZE_MAX / sizeof(int).
 * - Allocate `cap * sizeof(int)` into q->buf; return false on allocation failure.
 * - Initialize cap, head = 0, tail = 0, count = 0.
 * - Initialize `q->mtx`, `q->not_empty`, `q->not_full` with pthread_*_init(..., nullptr).
 * - Return true on success (unwind on any pthread_*_init failure).
 *
 * Requirements for bqueue_destroy:
 * - If q == nullptr || q->buf == nullptr, return.
 * - Destroy `not_full`, `not_empty`, `mtx`, free `q->buf`, and set `q->buf = nullptr`,
 *   `q->cap = 0`, `q->head = 0`, `q->tail = 0`, `q->count = 0`.
 */
static bool bqueue_init(BoundedQueue *q, size_t cap) {
  (void)q;
  (void)cap;
  return false; /* TODO: implement */
}

static void bqueue_destroy(BoundedQueue *q) {
  (void)q;
  /* TODO: implement */
}

/*
 * Requirements for bqueue_push:
 * - Lock `q->mtx`.
 * - Wait in a `while (q->count == q->cap)` loop on `q->not_full` (spurious-wakeup safe!).
 * - Store `item` at `q->buf[q->tail]`, advance `q->tail = (q->tail + 1) % q->cap`,
 *   increment `q->count`.
 * - Signal `q->not_empty` and unlock `q->mtx`.
 */
static void bqueue_push(BoundedQueue *q, int item) {
  (void)q;
  (void)item;
  /* TODO: implement */
}

/*
 * Requirements for bqueue_pop:
 * - Lock `q->mtx`.
 * - Wait in a `while (q->count == 0)` loop on `q->not_empty` (spurious-wakeup safe!).
 * - Read `item = q->buf[q->head]`, advance `q->head = (q->head + 1) % q->cap`,
 *   decrement `q->count`.
 * - Signal `q->not_full`, unlock `q->mtx`, and return `item`.
 */
static int bqueue_pop(BoundedQueue *q) {
  (void)q;
  return 0; /* TODO: implement */
}

/* ── Test Helpers ─────────────────────────────────────────────────────────── */

typedef struct {
  int write_fd;
} ChunkWriterArgs;

static void *chunked_pipe_writer(void *arg) {
  ChunkWriterArgs *a = (ChunkWriterArgs *)arg;
  const char *chunks[] = {"chunk1-", "chunk2-", "chunk3!"};
  struct timespec pause = {.tv_sec = 0, .tv_nsec = 5000000L}; /* 5 ms */

  for (size_t i = 0; i < 3; i++) {
    ssize_t w = rio_writen_robust(a->write_fd, chunks[i], strlen(chunks[i]));
    assert(w == (ssize_t)strlen(chunks[i]));
    nanosleep(&pause, nullptr);
  }
  close(a->write_fd);
  return nullptr;
}

typedef struct {
  BoundedQueue *q;
  int count;
  long sum;
} WorkerArgs;

static void *producer_worker(void *arg) {
  WorkerArgs *w = (WorkerArgs *)arg;
  for (int i = 1; i <= w->count; i++) {
    bqueue_push(w->q, i);
  }
  return nullptr;
}

static void *consumer_worker(void *arg) {
  WorkerArgs *w = (WorkerArgs *)arg;
  long local_sum = 0;
  for (int i = 0; i < w->count; i++) {
    local_sum += bqueue_pop(w->q);
  }
  w->sum = local_sum;
  return nullptr;
}

int main(void) {
  /* Drills 1 & 2: Short-count pipe assembly + EOF truncation */
  int fds[2] = {-1, -1};
  assert(pipe(fds) == 0);

  pthread_t writer_thr;
  ChunkWriterArgs wargs = {.write_fd = fds[1]};
  assert(pthread_create(&writer_thr, nullptr, chunked_pipe_writer, &wargs) == 0);

  char rx[32] = {0};
  ssize_t got = rio_readn_robust(fds[0], rx, 21);
  assert(got == 21);
  assert(memcmp(rx, "chunk1-chunk2-chunk3!", 21) == 0);

  /* Subsequent read after writer closes pipe must observe EOF (0 bytes) */
  assert(rio_readn_robust(fds[0], rx, 10) == 0);
  close(fds[0]);
  assert(pthread_join(writer_thr, nullptr) == 0);

  /* EOF short-read when fewer bytes than requested exist */
  int eof_pipe[2] = {-1, -1};
  assert(pipe(eof_pipe) == 0);
  assert(rio_writen_robust(eof_pipe[1], "hello", 5) == 5);
  close(eof_pipe[1]);
  char short_buf[16] = {0};
  assert(rio_readn_robust(eof_pipe[0], short_buf, sizeof(short_buf)) == 5);
  assert(memcmp(short_buf, "hello", 5) == 0);
  close(eof_pipe[0]);

  /* Drill 3: Concurrent producer/consumer BoundedQueue */
  assert(bqueue_init(nullptr, 4) == false);
  BoundedQueue q = {0};
  assert(bqueue_init(&q, 0) == false);
  assert(bqueue_init(&q, 4) == true);

  pthread_t prod[2];
  pthread_t cons[2];
  WorkerArgs pargs[2] = {{.q = &q, .count = 100, .sum = 0},
                         {.q = &q, .count = 100, .sum = 0}};
  WorkerArgs cargs[2] = {{.q = &q, .count = 100, .sum = 0},
                         {.q = &q, .count = 100, .sum = 0}};

  for (size_t i = 0; i < 2; i++) {
    assert(pthread_create(&prod[i], nullptr, producer_worker, &pargs[i]) == 0);
    assert(pthread_create(&cons[i], nullptr, consumer_worker, &cargs[i]) == 0);
  }
  for (size_t i = 0; i < 2; i++) {
    assert(pthread_join(prod[i], nullptr) == 0);
    assert(pthread_join(cons[i], nullptr) == 0);
  }

  assert(q.count == 0);
  assert(cargs[0].sum + cargs[1].sum == 10100L);
  bqueue_destroy(&q);
  assert(q.buf == nullptr);

  printf("[PASS] Exercise 57: Robust I/O & Pthreads Bounded Queue verified.\n");
  return EXIT_SUCCESS;
}
