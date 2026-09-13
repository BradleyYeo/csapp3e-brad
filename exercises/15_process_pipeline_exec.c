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
  pid_t parent_pid = getpid();
  assert(parent_pid > 0);

  pid_t pid = fork();
  if (pid < 0) {
    return false; // fork failed
  }

  if (pid == 0) {
    // Child process execution path
    pid_t my_pid = getpid();
    if (my_pid == parent_pid) {
      exit(EXIT_FAILURE); // Invariant check: Child PID must differ from parent
    }
    // Perform task and exit with status code
    exit(expected_exit_code);
  }

  // Parent process execution path
  int status = 0;
  pid_t reaped_pid = waitpid(pid, &status, 0);
  if (reaped_pid != pid) {
    return false;
  }

  // Verify child exited normally and check reported status byte
  if (!WIFEXITED(status) || WEXITSTATUS(status) != expected_exit_code) {
    return false;
  }

  return true;
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
  pid_t pid = fork();
  if (pid < 0) {
    return false;
  }

  if (pid == 0) {
    // Child sleeps indefinitely until killed
    while (true) {
      sleep(10);
    }
    exit(EXIT_SUCCESS);
  }

  // Parent sleeps briefly to allow child to schedule, then dispatches SIGTERM
  usleep(20000); // 20ms

  if (kill(pid, SIGTERM) != 0) {
    return false;
  }

  int status = 0;
  pid_t reaped = waitpid(pid, &status, 0);
  if (reaped != pid) {
    return false;
  }

  // Verify process was killed by a signal and that signal was SIGTERM
  if (!WIFSIGNALED(status) || WTERMSIG(status) != SIGTERM) {
    return false;
  }

  return true;
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
  int fd = open(temp_path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (fd < 0) {
    return false;
  }

  // Save current stdout
  int saved_stdout = dup(STDOUT_FILENO);
  if (saved_stdout < 0) {
    close(fd);
    return false;
  }

  // Redirect stdout to our open file
  if (dup2(fd, STDOUT_FILENO) < 0) {
    close(fd);
    close(saved_stdout);
    return false;
  }
  close(fd); // Original fd is no longer needed; STDOUT_FILENO now refers to the file

  // Writes to stdout now land directly in the file
  size_t len = strlen(payload);
  ssize_t written = write(STDOUT_FILENO, payload, len);

  // Restore original stdout
  dup2(saved_stdout, STDOUT_FILENO);
  close(saved_stdout);

  if (written != (ssize_t)len) {
    return false;
  }

  // Verify file contents by reading back
  int verify_fd = open(temp_path, O_RDONLY);
  if (verify_fd < 0) {
    return false;
  }

  char buf[64];
  memset(buf, 0, sizeof(buf));
  ssize_t bytes_read = read(verify_fd, buf, sizeof(buf) - 1);
  close(verify_fd);

  return (bytes_read == (ssize_t)len && strcmp(buf, payload) == 0);
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
  int pipefd[2];
  if (pipe(pipefd) != 0) {
    return false;
  }

  // Pipe to capture child2 stdout back into parent
  int capture_pipe[2];
  if (pipe(capture_pipe) != 0) {
    close(pipefd[0]);
    close(pipefd[1]);
    return false;
  }

  // Fork First Child (Producer: writes to pipefd[1])
  pid_t pid1 = fork();
  if (pid1 < 0) {
    return false;
  }

  if (pid1 == 0) {
    // Child 1: Connect stdout to pipefd[1]
    close(pipefd[0]);        // Close unused read end
    close(capture_pipe[0]);  // Unused in child 1
    close(capture_pipe[1]);  // Unused in child 1

    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);

    char *argv[] = {"echo", "kernel pipelines work", nullptr};
    execvp(argv[0], argv);
    // Only reachable if execvp fails
    exit(EXIT_FAILURE);
  }

  // Fork Second Child (Consumer: reads from pipefd[0], writes to capture_pipe[1])
  pid_t pid2 = fork();
  if (pid2 < 0) {
    return false;
  }

  if (pid2 == 0) {
    // Child 2: Connect stdin to pipefd[0], stdout to capture_pipe[1]
    close(pipefd[1]);        // Close unused write end
    close(capture_pipe[0]);  // Close unused capture read end

    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);

    dup2(capture_pipe[1], STDOUT_FILENO);
    close(capture_pipe[1]);

    char *argv[] = {"tr", "a-z", "A-Z", nullptr};
    execvp(argv[0], argv);
    // Only reachable if execvp fails
    exit(EXIT_FAILURE);
  }

  // Parent: Close pipefd ends so Child 2 observes EOF when Child 1 exits
  close(pipefd[0]);
  close(pipefd[1]);
  close(capture_pipe[1]); // Close capture write end so parent reads EOF

  // Read transformed output from capture pipe
  ssize_t total_read = 0;
  ssize_t bytes = 0;
  while ((bytes = read(capture_pipe[0], output_buf + total_read, max_output - 1 - total_read)) > 0) {
    total_read += bytes;
  }
  output_buf[total_read] = '\0';
  close(capture_pipe[0]);

  // Reap both children to eliminate zombies
  int status1 = 0;
  int status2 = 0;
  waitpid(pid1, &status1, 0);
  waitpid(pid2, &status2, 0);

  return (WIFEXITED(status1) && WEXITSTATUS(status1) == 0 &&
          WIFEXITED(status2) && WEXITSTATUS(status2) == 0);
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
