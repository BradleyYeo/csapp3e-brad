/*
 * Exercise 55: CPU Virtualization — Limited Direct Execution & Scheduling (OSTEP Ch. 6–9)
 *
 * Compile and run:
 *   make -C curriculum 55_ostep_cpu_sched_and_lde && ./curriculum/55_ostep_cpu_sched_and_lde
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

enum { STRIDE_TICKET_SCALE = 10000 };

typedef struct {
  uint32_t job_id;
  uint32_t priority;       /* 0 = lowest priority, num_levels - 1 = highest */
  uint32_t allotment_used; /* Ticks accumulated at current priority level */
  bool runnable;
} MlfqJob;

typedef struct {
  uint32_t job_id;
  uint32_t tickets;
  uint32_t stride;
  uint64_t pass;
  bool runnable;
} StrideJob;

/*
 * Drill 1: Monotonic Nanosecond Delta & Syscall Trap Cost (OSTEP Ch. 6 LDE)
 *
 * Limited Direct Execution runs user code directly on the CPU and enters the
 * kernel via a trap instruction for system calls. Measuring a 0-byte read(2)
 * isolates user->kernel->user mode-switch overhead.
 *
 * Requirements for timespec_diff_ns:
 * - Return 0 if `end` is earlier than `start`
 *   (`end.tv_sec < start.tv_sec` or equal seconds with `end.tv_nsec < start.tv_nsec`).
 * - Otherwise return `(uint64_t)sec_diff * 1000000000ULL + (uint64_t)nsec_diff`,
 *   borrowing 1 second (1000000000L ns) if `end.tv_nsec < start.tv_nsec`.
 *
 * Requirements for measure_syscall_cost_ns:
 * - Return false if fd < 0 || iterations == 0 || total_ns_out == nullptr.
 * - Capture `clock_gettime(CLOCK_MONOTONIC, &t0)`.
 * - Loop `iterations` times calling `read(fd, &dummy, 0)`; return false if any call < 0.
 * - Capture `clock_gettime(CLOCK_MONOTONIC, &t1)`.
 * - Store `timespec_diff_ns(t0, t1)` in `*total_ns_out` and return true.
 */
static uint64_t timespec_diff_ns(struct timespec start, struct timespec end) {
  (void)start;
  (void)end;
  return 0; /* TODO: implement */
}

static bool measure_syscall_cost_ns(int fd, size_t iterations, uint64_t *total_ns_out) {
  (void)fd;
  (void)iterations;
  (void)total_ns_out;
  return false; /* TODO: implement */
}

/*
 * Drill 2: Scheduling Metrics — Turnaround vs Response Time (OSTEP Ch. 7)
 *
 * T_turnaround = T_completion - T_arrival
 * T_response   = T_first_run  - T_arrival
 *
 * Requirements:
 * - Return false if turnaround_out == nullptr || response_out == nullptr.
 * - Return false if first_run < arrival || completion < first_run.
 * - Store turnaround and response times and return true.
 */
static bool sched_compute_metrics(uint32_t arrival, uint32_t first_run,
                                  uint32_t completion, uint32_t *turnaround_out,
                                  uint32_t *response_out) {
  (void)arrival;
  (void)first_run;
  (void)completion;
  (void)turnaround_out;
  (void)response_out;
  return false; /* TODO: implement */
}

/*
 * Drill 3: MLFQ Anti-Gaming Allotment & Priority Boost (OSTEP Ch. 8 Rules 4 & 5)
 *
 * Naive MLFQ reset a job's CPU budget whenever it yielded before its time slice
 * expired, letting an adversary game the scheduler by doing 1ms I/O repeatedly.
 * Rule 4 tracks cumulative allotment at a priority level regardless of yields.
 * Rule 5 prevents starvation by periodically boosting all jobs to the top queue.
 *
 * Requirements for mlfq_consume_slice:
 * - Return false if job == nullptr || level_allotments == nullptr ||
 *   num_levels == 0 || job->priority >= num_levels.
 * - Add `slice_ticks` to `job->allotment_used`.
 * - While `job->allotment_used >= level_allotments[job->priority]` (or single step
 *   if one slice exhausts current level):
 *     - If `job->priority > 0`: decrement `job->priority` by 1 and reset
 *       `job->allotment_used = 0` (unless your design carries remainder, stop after
 *       demotion and reset `allotment_used = 0` — test expects reset to 0 on demotion).
 *     - If `job->priority == 0`: clamp `job->allotment_used = level_allotments[0]`
 *       and stop.
 * - Return true.
 *
 * Requirements for mlfq_priority_boost:
 * - If jobs == nullptr || num_levels == 0, return.
 * - Set every job's `priority = num_levels - 1` and `allotment_used = 0`.
 */
static bool mlfq_consume_slice(MlfqJob *job, uint32_t slice_ticks,
                               const uint32_t *level_allotments,
                               uint32_t num_levels) {
  (void)job;
  (void)slice_ticks;
  (void)level_allotments;
  (void)num_levels;
  return false; /* TODO: implement */
}

static void mlfq_priority_boost(MlfqJob *jobs, size_t count, uint32_t num_levels) {
  (void)jobs;
  (void)count;
  (void)num_levels;
  /* TODO: implement */
}

/*
 * Drill 4: Deterministic Proportional-Share Stride Scheduler (OSTEP Ch. 9)
 *
 * Unlike randomized lottery scheduling, stride scheduling achieves exact
 * proportional CPU sharing over short intervals deterministically.
 *
 * Requirements for stride_job_init:
 * - Return false if job == nullptr || tickets == 0 || tickets > STRIDE_TICKET_SCALE.
 * - Set job_id, tickets, `stride = STRIDE_TICKET_SCALE / tickets`, `pass = 0`, `runnable = true`.
 * - Return true.
 *
 * Requirements for stride_pick_and_advance:
 * - Return nullptr if jobs == nullptr || count == 0.
 * - Find the runnable job with the minimum `pass` value (on tie, prefer smaller `job_id`).
 * - If no job is runnable, return nullptr.
 * - Advance the selected job's `pass += selected->stride` and return `selected`.
 */
static bool stride_job_init(StrideJob *job, uint32_t job_id, uint32_t tickets) {
  (void)job;
  (void)job_id;
  (void)tickets;
  return false; /* TODO: implement */
}

static StrideJob *stride_pick_and_advance(StrideJob *jobs, size_t count) {
  (void)jobs;
  (void)count;
  return nullptr; /* TODO: implement */
}

int main(void) {
  /* Drill 1: Nanosecond delta with borrow + syscall trap timing */
  struct timespec s1 = {.tv_sec = 10, .tv_nsec = 900000000L};
  struct timespec e1 = {.tv_sec = 11, .tv_nsec = 100000000L};
  assert(timespec_diff_ns(s1, e1) == 200000000ULL);
  assert(timespec_diff_ns(e1, s1) == 0ULL);

  char tmp_path[] = "/tmp/ostep_lde_XXXXXX";
  int fd = mkstemp(tmp_path);
  assert(fd >= 0);
  unlink(tmp_path);

  uint64_t total_ns = 0;
  assert(measure_syscall_cost_ns(-1, 100, &total_ns) == false);
  assert(measure_syscall_cost_ns(fd, 0, &total_ns) == false);
  assert(measure_syscall_cost_ns(fd, 1000, nullptr) == false);
  assert(measure_syscall_cost_ns(fd, 1000, &total_ns) == true);
  assert(total_ns > 0);
  close(fd);

  /* Drill 2: Turnaround & Response metrics */
  uint32_t turnaround = 0;
  uint32_t response = 0;
  assert(sched_compute_metrics(10, 5, 20, &turnaround, &response) == false);
  assert(sched_compute_metrics(10, 15, 12, &turnaround, &response) == false);
  assert(sched_compute_metrics(0, 10, 30, &turnaround, &response) == true);
  assert(turnaround == 30 && response == 10);
  assert(sched_compute_metrics(5, 8, 20, &turnaround, &response) == true);
  assert(turnaround == 15 && response == 3);

  /* Drill 3: MLFQ anti-gaming Rule 4 + priority boost Rule 5 */
  const uint32_t allotments[3] = {40, 20, 10}; /* Level 2 (top) gets 10 ticks */
  MlfqJob gamer = {.job_id = 1, .priority = 2, .allotment_used = 0, .runnable = true};
  assert(mlfq_consume_slice(&gamer, 3, allotments, 3) == true);
  assert(gamer.priority == 2 && gamer.allotment_used == 3);
  assert(mlfq_consume_slice(&gamer, 3, allotments, 3) == true);
  assert(gamer.priority == 2 && gamer.allotment_used == 6);
  assert(mlfq_consume_slice(&gamer, 3, allotments, 3) == true);
  assert(gamer.priority == 2 && gamer.allotment_used == 9);
  /* 4th 3-tick burst pushes used to 12 >= 10 -> demoted to priority 1, allotment reset */
  assert(mlfq_consume_slice(&gamer, 3, allotments, 3) == true);
  assert(gamer.priority == 1 && gamer.allotment_used == 0);

  MlfqJob starved = {.job_id = 2, .priority = 0, .allotment_used = 35, .runnable = true};
  assert(mlfq_consume_slice(&starved, 10, allotments, 3) == true);
  assert(starved.priority == 0 && starved.allotment_used == 40);

  MlfqJob flock[2] = {gamer, starved};
  mlfq_priority_boost(flock, 2, 3);
  assert(flock[0].priority == 2 && flock[0].allotment_used == 0);
  assert(flock[1].priority == 2 && flock[1].allotment_used == 0);

  /* Drill 4: Deterministic Stride Scheduling (100 : 50 : 150 -> 2 : 1 : 3 ratio) */
  StrideJob sjobs[3] = {0};
  assert(stride_job_init(&sjobs[0], 1, 0) == false);
  assert(stride_job_init(&sjobs[0], 1, 100) == true);
  assert(stride_job_init(&sjobs[1], 2, 50) == true);
  assert(stride_job_init(&sjobs[2], 3, 150) == true);

  uint32_t runs[3] = {0, 0, 0};
  for (int q = 0; q < 60; q++) {
    StrideJob *picked = stride_pick_and_advance(sjobs, 3);
    assert(picked != nullptr);
    runs[picked->job_id - 1]++;
  }
  assert(runs[0] == 20 && runs[1] == 10 && runs[2] == 30);

  sjobs[0].runnable = false;
  sjobs[1].runnable = false;
  sjobs[2].runnable = false;
  assert(stride_pick_and_advance(sjobs, 3) == nullptr);

  printf("[PASS] Exercise 55: OSTEP CPU Scheduling & LDE verified.\n");
  return EXIT_SUCCESS;
}
