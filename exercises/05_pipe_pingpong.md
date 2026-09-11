# 05: UNIX Pipes Ping-Pong Benchmark and IPC Guide

An anonymous pipe is the fundamental unidirectional Inter-Process Communication (IPC) primitive in UNIX systems. This guide covers kernel pipe mechanics, process duplication with `fork(2)`, descriptor table inheritance, context switching latency, and high-precision performance benchmarking.

## Curriculum Reading Sequence
- Layer 01: [01: Systems C Fundamentals, Syntax, and Core Concepts](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/01_FUNDAMENTALS.md)
- Layer 02: [02: Hands-On Practice Exercises and Deliberate Practice Drills](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/02_README.md)
- Layer 03: [03: Fixed-Size Bump Allocator Architecture and Implementation Guide](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/03_bump_allocator.md)
- Layer 04: [04: Memory Debugging, Sanitizers, and Defect Remediation Manual](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/04_MEMORY_DEBUGGING.md)
- Layer 05: [05: UNIX Pipes Ping-Pong Benchmark and IPC Guide](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/05_pipe_pingpong.md) (Current Document)
- Layer 06: [06: Valgrind Architecture, Memcheck Diagnostics, and Memory Profiling Guide](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/06_valgrind_fundamentals.md)

---

# Kernel IPC Architecture and Anonymous Pipes

## Unidirectional Byte Stream Mechanics

An anonymous pipe created via `pipe(int pipefd[2])` allocates a pair of file descriptors linked to an in-kernel circular memory buffer:
- `pipefd[0]`: Read end of the pipe (read-only).
- `pipefd[1]`: Write end of the pipe (write-only).
- Buffer Capacity: On Linux, the default pipe buffer capacity is 65,536 bytes (16 memory pages of 4 KB each).
- Byte Stream Semantics: Pipes do not preserve message boundaries. Data written across multiple calls to `write` coalesces into a continuous byte stream read by `read`.

```
Write End (pipefd[1]) ---> [ Kernel Circular Buffer (64 KB) ] ---> Read End (pipefd[0])
```

## Bidirectional Full-Duplex IPC Topology

Because a single pipe is strictly unidirectional, bidirectional communication between two processes requires two separate pipes:
- `p2c[2]` (Parent-to-Child Pipe): Parent writes to `p2c[1]`; child reads from `p2c[0]`.
- `c2p[2]` (Child-to-Parent Pipe): Child writes to `c2p[1]`; parent reads from `c2p[0]`.

```
+------------------+                    +------------------+
|  Parent Process  | -- p2c[1] (write) -> | Kernel Buffer A  | -- p2c[0] (read) -> |  Child Process   |
|                  | <-- c2p[0] (read) -- | Kernel Buffer B  | <- c2p[1] (write) - |                  |
+------------------+                    +------------------+
```

---

# Process Duplication and File Descriptor Tables

## Fork Semantics and Descriptor Inheritance

When `fork(2)` executes, the kernel duplicates the calling process's entire address space via Copy-On-Write (COW) and creates an identical copy of the parent's file descriptor table:
- Both parent and child now hold open descriptors pointing to the exact same underlying kernel pipe files.
- Refcount Increment: The reference count of both the read and write ends of each pipe is incremented to 2.

```
Process Tables Immediately After fork():

Parent Process Descriptors:         Kernel Open File Objects:
  p2c[0] (Read)   ---------------------> [ p2c Pipe Read Object  ] (refcount = 2)
  p2c[1] (Write)  ---------------------> [ p2c Pipe Write Object ] (refcount = 2)
  c2p[0] (Read)   ---------------------> [ c2p Pipe Read Object  ] (refcount = 2)
  c2p[1] (Write)  ---------------------> [ c2p Pipe Write Object ] (refcount = 2)
                                                 ^
Child Process Descriptors:                       |
  p2c[0] (Read)   -------------------------------+
  p2c[1] (Write)  -------------------------------+
  c2p[0] (Read)   -------------------------------+
  c2p[1] (Write)  -------------------------------+
```

## The Unused Descriptor Invariant (Jimmy Koppel & Daniel Jackson)

Every process must immediately close the pipe ends it does not intend to use.

### Why Reader Must Close Write End
- In UNIX, `read()` on a pipe returns `0` (EOF) only when all open write descriptors referencing that pipe have been closed across the entire system.
- If the child reads from `p2c[0]` without closing its inherited copy of `p2c[1]`, the child itself keeps the write end alive.
- If the parent terminates or encounters an error, the child will never observe EOF and will hang forever in a blocked `read()`, causing a permanent deadlock.

### Why Writer Should Close Read End
- Closing unused read ends prevents file descriptor leaks and enforces the concept that each pipe end represents a unidirectional communication channel.

### Correct Cleanup Topology
- In Parent:
  ```c
  close(p2c[0]); /* Close unused read end of parent-to-child pipe */
  close(c2p[1]); /* Close unused write end of child-to-parent pipe */
  ```
- In Child:
  ```c
  close(p2c[1]); /* Close unused write end of parent-to-child pipe */
  close(c2p[0]); /* Close unused read end of child-to-parent pipe */
  ```

---

# Context Switching and Latency Breakdown

The ping-pong exchange program forces the operating system kernel to switch execution between two processes as fast as possible.

## Anatomy of a Single Exchange Round-Trip

One complete ping-pong round-trip consists of the following phases:
- Phase 1 (Parent Write): Parent executes `write(p2c[1], &byte, 1)`. Transitions from user mode to kernel mode via `syscall`.
- Phase 2 (Kernel Copy & Wakeup): Kernel copies the byte into the `p2c` circular buffer, marks the sleeping child process runnable, and moves it to the CPU run queue.
- Phase 3 (Parent Wait): Parent immediately calls `read(c2p[0], &byte, 1)`. Because `c2p` is currently empty, the kernel suspends the parent, transitioning it from RUNNING to BLOCKED (waiting on I/O).
- Phase 4 (CPU Context Switch):
  - Linux scheduler (`sched_switch`) selects the child process.
  - Architectural Context Switch: Saves parent CPU registers, updates page table pointer (reloading `CR3` register on x86_64), invalidates non-global TLB entries, and loads child CPU registers.
  - Cache Thrashing: The CPU L1/L2 caches now contain the parent's working set, forcing cache warm-up for the child.
- Phase 5 (Child Wakeup & Read): Child exits `read()` system call into user space, reading the byte.
- Phase 6 (Child Reply): Child calls `write(c2p[1], &byte, 1)` and immediately calls `read(p2c[0], &byte, 1)`, blocking itself.
- Phase 7 (Second Context Switch): Kernel switches execution back to the parent process.

```
Total Transitions Per Exchange:
- 4 User <-> Kernel System Call Transitions (2 writes, 2 reads).
- 2 Full Process Context Switches (Parent -> Child -> Parent).
- 2 TLB / Address Space Invalidations.
```

---

# Performance Measurement and Benchmarking

## High-Precision Monotonic Timing

Timing system calls and context switches requires a clock source that cannot jump backwards:
- Avoid `gettimeofday(2)`: Wall-clock time is subject to Network Time Protocol (NTP) adjustments and clock stepping.
- Use `clock_gettime(CLOCK_MONOTONIC, &ts)`: Represents steady monotonic time since an unspecified starting point (e.g. system boot).

### Elapsed Nanoseconds Calculation
```c
struct timespec start, end;
clock_gettime(CLOCK_MONOTONIC, &start);

/* Execute N ping-pong exchanges */

clock_gettime(CLOCK_MONOTONIC, &end);

double elapsed_seconds = (double)(end.tv_sec - start.tv_sec) +
                         (double)(end.tv_nsec - start.tv_nsec) / 1e9;
```

## Performance Metrics Formulation

- Total Exchanges ($N$): Typically 100,000 round-trips.
- Throughput:
  $$\text{Exchanges per Second} = \frac{N}{\Delta t}$$
- Round-Trip Latency (RTT):
  $$\text{Latency}_{\text{RTT}} = \frac{\Delta t}{N} \times 10^6 \quad (\mu\text{s})$$
- One-Way Transit / Context Switch Time:
  $$\text{Latency}_{\text{one-way}} = \frac{\text{Latency}_{\text{RTT}}}{2} \quad (\mu\text{s})$$

---

# Complete C23 Reference Implementation

Here is the complete reference implementation with inline annotations.

```c
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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_ITERATIONS 100000

/*
 * Reads exactly 1 byte from fd, defensively retrying if interrupted by signals (EINTR).
 *
 * Returns:
 * - true on success.
 * - false on EOF (read returns 0) or unrecoverable error.
 */
static bool safe_read_byte(int fd, uint8_t *byte_out) {
  while (true) {
    ssize_t n = read(fd, byte_out, 1);
    if (n == 1) {
      return true;
    }
    if (n == 0) {
      return false; /* EOF */
    }
    if (n == -1) {
      if (errno == EINTR) {
        continue; /* System call interrupted by signal; retry */
      }
      return false; /* Read error */
    }
  }
}

/*
 * Writes exactly 1 byte to fd, defensively retrying if interrupted by signals (EINTR).
 *
 * Returns:
 * - true on success.
 * - false on error.
 */
static bool safe_write_byte(int fd, uint8_t byte) {
  while (true) {
    ssize_t n = write(fd, &byte, 1);
    if (n == 1) {
      return true;
    }
    if (n == -1) {
      if (errno == EINTR) {
        continue; /* System call interrupted by signal; retry */
      }
      return false; /* Write error */
    }
  }
}

/*
 * Child process routine:
 * - Reads byte from parent via read_fd.
 * - Increments byte payload.
 * - Sends byte back to parent via write_fd.
 * - Repeats for specified iteration count.
 */
static void run_child(int read_fd, int write_fd, size_t iterations) {
  uint8_t payload = 0;

  for (size_t i = 0; i < iterations; ++i) {
    if (!safe_read_byte(read_fd, &payload)) {
      perror("child read failed");
      exit(EXIT_FAILURE);
    }

    payload++;

    if (!safe_write_byte(write_fd, payload)) {
      perror("child write failed");
      exit(EXIT_FAILURE);
    }
  }

  /* Close descriptors and terminate cleanly */
  close(read_fd);
  close(write_fd);
  exit(EXIT_SUCCESS);
}

/*
 * Parent process routine:
 * - Sends byte to child via write_fd.
 * - Waits for response byte from child via read_fd.
 * - Repeats for specified iteration count.
 */
static void run_parent(int write_fd, int read_fd, size_t iterations) {
  uint8_t payload = 0x42;

  for (size_t i = 0; i < iterations; ++i) {
    if (!safe_write_byte(write_fd, payload)) {
      perror("parent write failed");
      exit(EXIT_FAILURE);
    }

    if (!safe_read_byte(read_fd, &payload)) {
      perror("parent read failed");
      exit(EXIT_FAILURE);
    }
  }

  /* Close descriptors after completing all exchanges */
  close(write_fd);
  close(read_fd);
}

int main(int argc, char **argv) {
  size_t iterations = DEFAULT_ITERATIONS;
  if (argc > 1) {
    long parsed = atol(argv[1]);
    if (parsed > 0) {
      iterations = (size_t)parsed;
    }
  }

  printf("Benchmarking UNIX pipe ping-pong with %zu exchanges...\n", iterations);

  /*
   * Step 1: Allocate two unidirectional pipes.
   * p2c: Parent-to-Child (parent writes [1], child reads [0])
   * c2p: Child-to-Parent (child writes [1], parent reads [0])
   */
  int p2c[2];
  int c2p[2];

  if (pipe(p2c) != 0 || pipe(c2p) != 0) {
    perror("pipe creation failed");
    return EXIT_FAILURE;
  }

  /*
   * Step 2: Clone process address space.
   */
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork failed");
    return EXIT_FAILURE;
  }

  if (pid == 0) {
    /*
     * Child Process Context:
     * Invariant: Close unused descriptor ends immediately!
     * Child reads from p2c[0], writes to c2p[1].
     */
    close(p2c[1]); /* Close unused write end */
    close(c2p[0]); /* Close unused read end */

    run_child(p2c[0], c2p[1], iterations);
    /* run_child terminates with exit() */
  }

  /*
   * Parent Process Context:
   * Invariant: Close unused descriptor ends immediately!
   * Parent writes to p2c[1], reads from c2p[0].
   */
  close(p2c[0]); /* Close unused read end */
  close(c2p[1]); /* Close unused write end */

  /*
   * Step 3: High-resolution benchmark timing.
   */
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  run_parent(p2c[1], c2p[0], iterations);

  clock_gettime(CLOCK_MONOTONIC, &end);

  /*
   * Step 4: Reclaim child process to prevent zombie states.
   */
  int status = 0;
  waitpid(pid, &status, 0);
  assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);

  /*
   * Step 5: Metric calculation and reporting.
   */
  double elapsed_sec = (double)(end.tv_sec - start.tv_sec) +
                       (double)(end.tv_nsec - start.tv_nsec) / 1e9;
  double exchanges_per_sec = (double)iterations / elapsed_sec;
  double roundtrip_us = (elapsed_sec * 1e6) / (double)iterations;
  double oneway_us = roundtrip_us / 2.0;

  printf("Benchmark Results:\n");
  printf("  Total Exchanges:      %zu round-trips\n", iterations);
  printf("  Elapsed Time:         %.4f seconds\n", elapsed_sec);
  printf("  Throughput:           %.2f exchanges/second\n", exchanges_per_sec);
  printf("  Avg Round-Trip (RTT): %.3f us\n", roundtrip_us);
  printf("  Avg One-Way Transit:  %.3f us (Context Switch + Pipe Transit)\n", oneway_us);

  printf("[PASS] Exercise 12: UNIX Pipes Ping-Pong Benchmark verified.\n");
  return EXIT_SUCCESS;
}
```

---

# Active Recall and Mental Model Checks

- Why is a single anonymous pipe insufficient for two processes to take turns sending messages to each other?
- If the child process does not close `p2c[1]`, what happens if the parent unexpectedly crashes during an exchange?
- What is the difference between `CLOCK_MONOTONIC` and `CLOCK_REALTIME`? Why is `CLOCK_REALTIME` hazardous for profiling latency?
- If a benchmark achieves 50,000 exchanges per second, how many context switches occur each second?
- Why must the parent invoke `waitpid(pid, ...)` after the loop completes? What operating system resource is leaked if `waitpid` is omitted?
