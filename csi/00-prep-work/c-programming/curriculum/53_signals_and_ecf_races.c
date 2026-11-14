/*
 * Exercise 56: Signals, Coalesced Reaping & ECF Race Conditions (CS:APP Ch. 8 / shlab)
 *
 * Compile and run:
 *   make -C curriculum 56_signals_and_ecf_races && ./curriculum/56_signals_and_ecf_races
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
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  pid_t pids[16];
  size_t count;
} JobTable;

static volatile sig_atomic_t g_reaped_count = 0;

/*
 * Drill 1: Non-Blocking Coalesced Zombie Reaping
 *
 * POSIX signals are NOT queued: if 3 children terminate while SIGCHLD is
 * blocked or while the handler is already running, only 1 SIGCHLD may arrive.
 * A handler or reaper MUST loop with WNOHANG until no zombies remain.
 *
 * Requirements:
 * - Loop calling `waitpid(-1, &status, WNOHANG)` while return value > 0.
 * - For each reaped pid (> 0), if reaped_pids != nullptr and total < max_pids,
 *   store `reaped_pids[total] = pid`. Increment `total`.
 * - If waitpid returns 0 or (-1 with errno == ECHILD), stop and return `total`.
 * - If waitpid fails with any other errno (other than EINTR), return -1.
 */
static int reap_all_zombies_nonblocking(pid_t *reaped_pids, size_t max_pids) {
  (void)reaped_pids;
  (void)max_pids;
  return -1; /* TODO: implement */
}

/*
 * Drill 2: Async-Signal-Safe SIGCHLD Handler Preserving errno
 *
 * Requirements:
 * - Save `int saved_errno = errno;` at handler entry.
 * - Loop `while (waitpid(-1, nullptr, WNOHANG) > 0)` incrementing `g_reaped_count`.
 *   (Note: only call async-signal-safe functions such as waitpid here — NEVER printf/malloc!)
 * - Restore `errno = saved_errno;` before returning so interrupted syscalls/libc
 *   code in main never see `ECHILD` clobbering their `errno`.
 */
static void sigchld_preserving_handler(int sig) {
  (void)sig;
  /* TODO: implement */
}

/*
 * Drill 3: Blocking SIGCHLD Across fork() + Job Table Registration
 *
 * Classic CS:APP Ch. 8 Race Condition:
 *   If parent calls fork(), child exits immediately, and SIGCHLD fires BEFORE
 *   parent adds child's PID to `jobs`, the signal handler tries to delete a job
 *   that has not been inserted yet (and parent later adds a zombie/dead PID)!
 *
 * Requirements:
 * - Return false if jobs == nullptr || jobs->count >= 16.
 * - Build a sigset_t containing SIGCHLD; block it via `sigprocmask(SIG_BLOCK, &mask, &prev)`.
 * - Call `fork()`.
 *   - On fork() < 0: restore `sigprocmask(SIG_SETMASK, &prev, nullptr)` and return false.
 *   - On child (pid == 0): restore `sigprocmask(SIG_SETMASK, &prev, nullptr)`, then `_exit(0)`.
 *   - On parent (pid > 0): while SIGCHLD is STILL blocked, insert `pid` into
 *     `jobs->pids[jobs->count++] = pid;` and set `*out_child_pid = pid` (if non-null).
 *     Only AFTER updating `jobs`, restore `sigprocmask(SIG_SETMASK, &prev, nullptr)`.
 * - Return true.
 */
static bool spawn_and_register_job_safe(JobTable *jobs, pid_t *out_child_pid) {
  (void)jobs;
  (void)out_child_pid;
  return false; /* TODO: implement */
}

static bool contains_pid(const pid_t *arr, size_t n, pid_t target) {
  for (size_t i = 0; i < n; i++) {
    if (arr[i] == target) {
      return true;
    }
  }
  return false;
}

int main(void) {
  /* Drill 1: Coalesced non-blocking zombie reaping */
  pid_t spawned[3] = {0};
  for (size_t i = 0; i < 3; i++) {
    pid_t pid = fork();
    assert(pid >= 0);
    if (pid == 0) {
      _exit((int)i);
    }
    spawned[i] = pid;
  }

  pid_t reaped[8] = {0};
  int total_reaped = 0;
  for (int attempt = 0; attempt < 40 && total_reaped < 3; attempt++) {
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 10000000L}; /* 10 ms */
    nanosleep(&ts, nullptr);
    int n = reap_all_zombies_nonblocking(reaped + total_reaped,
                                         8 - (size_t)total_reaped);
    assert(n >= 0);
    total_reaped += n;
  }
  assert(total_reaped == 3);
  for (size_t i = 0; i < 3; i++) {
    assert(contains_pid(reaped, 3, spawned[i]));
  }
  assert(reap_all_zombies_nonblocking(reaped, 8) == 0);

  /* Drill 2: errno preservation in sigchld_preserving_handler */
  errno = EINVAL;
  sigchld_preserving_handler(SIGCHLD); /* waitpid fails with ECHILD; handler must restore EINVAL */
  assert(errno == EINVAL);

  /* Install SIGCHLD handler and verify async reaping */
  struct sigaction sa = {0};
  struct sigaction old_sa = {0};
  sa.sa_handler = sigchld_preserving_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  assert(sigaction(SIGCHLD, &sa, &old_sa) == 0);

  g_reaped_count = 0;
  JobTable jobs = {0};
  for (int i = 0; i < 4; i++) {
    pid_t child = -1;
    assert(spawn_and_register_job_safe(&jobs, &child) == true);
    assert(child > 0);
    assert(jobs.pids[i] == child);
  }
  assert(jobs.count == 4);

  for (int attempt = 0; attempt < 50 && g_reaped_count < 4; attempt++) {
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 10000000L};
    nanosleep(&ts, nullptr);
  }
  assert(g_reaped_count == 4);

  assert(sigaction(SIGCHLD, &old_sa, nullptr) == 0);

  printf("[PASS] Exercise 56: Signals & ECF Race Conditions verified.\n");
  return EXIT_SUCCESS;
}
