/*
 * Exercise 15: Process Lifecycle, IPC Pipelines, and Signal Management
 *
 * Covers system calls:
 * - fork(), exit(), wait() / waitpid(), kill(), getpid(), sleep()
 * - exec() / execvp(), pipe(), dup() / dup2(), close(), read(), write()
 *
 * Compile and run:
 *   clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 15_process_pipeline_exec 15_process_pipeline_exec.c && ./15_process_pipeline_exec
 */
#define _POSIX_C_SOURCE 200809L
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#include <stdbool.h>
#include <stdalign.h>
#ifndef nullptr
#define nullptr NULL
#endif
#endif

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Drill 1: Process Creation, Exit Status Encoding, and Reaping
 *
 * Demonstrates:
 * - fork(): Creates child with identical Copy-on-Write (COW) address space.
 * - getpid(): Resolves current process identifier.
 * - exit(): Terminates child and sets exit code in process control block.
 * - waitpid(): Blocks until specified child terminates, reaps zombie, extracts exit status.
 */
static bool test_fork_and_wait(int expected_exit_code) {
  (void)expected_exit_code;
  return false; /* TODO: implement */
}

/*
 * Drill 2: Inter-Process Signal Dispatch and Process Suspension
 *
 * Demonstrates:
 * - sleep(): Suspends process execution for specified duration.
 * - kill(): Dispatches kernel signals (SIGTERM) to a target process PID.
 * - waitpid(): Reaps signaled child and inspects signal termination via WIFSIGNALED.
 */
static bool test_signal_kill(void) {
  return false; /* TODO: implement */
}

/*
 * Drill 3: File Descriptor Duplication and Redirection
 *
 * Demonstrates:
 * - dup(): Clones a descriptor to save a backup handle.
 * - dup2(): Atomically closes target descriptor and clones source to target.
 * - close(): Decrements reference count on open file table entry.
 */
static bool test_fd_redirection(const char *temp_path, const char *payload) {
  (void)temp_path;
  (void)payload;
  return false; /* TODO: implement */
}

/*
 * Drill 4: Pipelined Execution with Two Children (cmd1 | cmd2)
 *
 * Simulates the shell pipeline:
 *   echo "hello systems world" | tr "a-z" "A-Z"
 *
 * Demonstrates:
 * - pipe(): Unidirectional kernel buffer.
 * - The Unused Descriptor Invariant: All unused write ends must be closed in all
 *   processes, otherwise reader hangs indefinitely waiting for EOF.
 * - execvp(): Overlays process address space with new program executable.
 */
static bool test_pipeline_execution(char *output_buf, size_t max_output) {
  (void)output_buf;
  (void)max_output;
  return false; /* TODO: implement */
}

int main(void) {
  printf("Running Exercise 15: Process Lifecycle, IPC Pipelines & Signals...\n");

  // Test 1: Fork, GetPID, Exit, and Wait
  assert(test_fork_and_wait(42) == true);
  printf("  [PASS] Drill 1: fork, getpid, exit, and waitpid verified.\n");

  // Test 2: Signal dispatch and sleep
  assert(test_signal_kill() == true);
  printf("  [PASS] Drill 2: sleep and kill (SIGTERM) verified.\n");

  // Test 3: File descriptor redirection with dup/dup2
  char temp_path[] = "/tmp/test_dup_XXXXXX";
  int temp_fd = mkstemp(temp_path);
  assert(temp_fd >= 0);
  close(temp_fd);

  assert(test_fd_redirection(temp_path, "redirection_payload_ok\n") == true);
  unlink(temp_path);
  printf("  [PASS] Drill 3: dup, dup2, open, write, and read redirection verified.\n");

  // Test 4: Pipeline execution (cmd1 | cmd2) with execvp
  char pipeline_output[128];
  memset(pipeline_output, 0, sizeof(pipeline_output));
  assert(test_pipeline_execution(pipeline_output, sizeof(pipeline_output)) == true);
  assert(strstr(pipeline_output, "KERNEL PIPELINES WORK") != nullptr);
  printf("  [PASS] Drill 4: pipe, execvp, and 2-child pipeline verified.\n");

  printf("[PASS] Exercise 15: All process and pipeline system calls verified successfully.\n");
  return EXIT_SUCCESS;
}
