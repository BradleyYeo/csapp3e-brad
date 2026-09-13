# Stage 7: Exercise 12 - UNIX Pipes Ping-Pong Benchmark and Latency Profiling

A guide explaining unidirectional and bidirectional pipes (`pipe`), descriptor table hygiene, `EINTR` signal recovery, CPU context switching latency, and high-resolution timing (`clock_gettime`) for C beginners.

# Conceptual Foundations and Mental Model

## What Is a UNIX Pipe?
- A pipe is a unidirectional IPC (Inter-Process Communication) channel allocated directly in the kernel's memory space.
- A standard pipe consists of a 64KB circular ring buffer managed by the kernel.
- System call `pipe(int pipefd[2])`:
  - Allocates two file descriptors:
    - `pipefd[0]`: Read end (data emerges here).
    - `pipefd[1]`: Write end (data is inserted here).
  - Data is consumed on a First-In, First-Out (FIFO) basis.

## Full-Duplex vs Half-Duplex Communication
- A single pipe is strictly half-duplex (data flows in one direction: from write end to read end).
- Attempting to use a single pipe for two-way communication between parent and child causes a race condition: parent writes a byte and immediately reads its own byte back before the child ever sees it!
- To achieve full-duplex ping-pong:
  - Pipe 1 (`p2c`): Parent-to-Child channel. Parent writes to `p2c[1]`; child reads from `p2c[0]`.
  - Pipe 2 (`c2p`): Child-to-Parent channel. Child writes to `c2p[1]`; parent reads from `c2p[0]`.

## The Closing Unused End Invariant
- Immediately after `fork()`, both parent and child inherit duplicate copies of all 4 pipe descriptors.
- Invariant: Every process MUST immediately close the descriptor ends it does not intend to use:
  - Child closes `p2c[1]` (write end) and `c2p[0]` (read end).
  - Parent closes `p2c[0]` (read end) and `c2p[1]` (write end).
- Why?
  - If child does not close `p2c[1]`, the read end `p2c[0]` will never receive EOF when parent terminates because a write descriptor is still held open by the child itself.
  - If a reader terminates, a writer writing to the pipe receives `SIGPIPE` only if all reader descriptors are closed.

## Signal Interruption: Handling EINTR
- When a process blocks on a slow system call (`read`, `write`, `sleep`), the kernel may interrupt it if a signal arrives (e.g. `SIGCHLD`, `SIGALRM`).
- The call returns `-1` with `errno == EINTR`.
- Defensive systems code wraps blocking calls in an retry loop:
  ```c
  if (n == -1 && errno == EINTR) continue;
  ```

---

# Line-by-Line Code Breakdown

## Function: safe_read_byte

```c
static bool safe_read_byte(int fd, uint8_t *byte_out) {
  if (fd < 0 || byte_out == nullptr) {
    return false;
  }
  while (true) {
    ssize_t n = read(fd, byte_out, 1);
    if (n == 1) {
      return true;
    }
    if (n == 0) {
      return false; // EOF
    }
    if (n == -1) {
      if (errno == EINTR) {
        continue;
      }
      return false;
    }
  }
}
```

### Tri-State System Call Handling
- `n == 1`: Exactly 1 byte was read into `*byte_out`; success.
- `n == 0`: End-of-file condition. All writers have closed the pipe.
- `n == -1`: Error. Retries if interrupted by signal (`EINTR`), otherwise returns `false`.

---

## Function: calculate_metrics

```c
static bool calculate_metrics(struct timespec start, struct timespec end, size_t iterations, BenchmarkMetrics *out) {
  if (out == nullptr || iterations == 0) {
    return false;
  }

  double elapsed = (double)(end.tv_sec - start.tv_sec) +
                   (double)(end.tv_nsec - start.tv_nsec) / 1e9;
  if (elapsed <= 0.0) {
    return false;
  }

  out->elapsed_seconds = elapsed;
  out->exchanges_per_second = (double)iterations / elapsed;
  out->roundtrip_latency_us = (elapsed * 1e6) / (double)iterations;
  out->oneway_transit_us = out->roundtrip_latency_us / 2.0;
  return true;
}
```

### High-Resolution Timing with CLOCK_MONOTONIC
- `clock_gettime(CLOCK_MONOTONIC, &ts)`: Returns monotonic time that never jumps backward during NTP time synchronization adjustments.
- Nanosecond resolution allows precise measurement of microsecond-scale context switch latencies.

---

# Memory Layout Visualization

Full-duplex dual-pipe IPC architecture:

```
+---------------------+                            +---------------------+
|   Parent Process    |                            |    Child Process    |
|                     |                            |                     |
|  [ fd: p2c[1] ] ----+---> [ Pipe 1: p2c ] ------>+---> [ fd: p2c[0] ]  |
|  (Parent Writer)    |     (Kernel Ring Buffer)   |     (Child Reader)  |
|                     |                            |                     |
|  [ fd: c2p[0] ] <---+---< [ Pipe 2: c2p ] <------+---< [ fd: c2p[1] ]  |
|  (Parent Reader)    |     (Kernel Ring Buffer)   |     (Child Writer)  |
+---------------------+                            +---------------------+
```

Step-by-step Ping-Pong exchange:

```
Time T0: Parent writes byte 0x42 to p2c[1].
Time T1: Parent blocks in read(c2p[0]).
Time T2: OS scheduler context-switches CPU to Child process.
Time T3: Child wakes up from read(p2c[0]), receives 0x42.
Time T4: Child increments byte to 0x43 and writes to c2p[1].
Time T5: Child blocks in read(p2c[0]).
Time T6: OS scheduler context-switches CPU back to Parent process.
Time T7: Parent wakes up from read(c2p[0]), receives 0x43. Round-trip complete!
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: Why is a single pipe insufficient for two-way bidirectional communication between parent and child?
- Question 2: Why must a reader process close its inherited copy of the pipe's write end (`pipefd[1]`)?
- Question 3: What does `errno == EINTR` mean when returned from `read()` or `write()`, and why should the call be retried?

## Drill: Hands-On Code Validation
- Run the ping-pong benchmark with varying iteration counts:
  ```bash
  clang -Wall -Wextra -pedantic -g -o 12_pipe_pingpong 12_pipe_pingpong.c
  ./12_pipe_pingpong 1000
  ./12_pipe_pingpong 50000
  ```
- Observe how throughput (exchanges/sec) and round-trip time (RTT in microseconds) change as the iteration count increases.
- Confirm that average one-way transit time is between 2 and 10 microseconds.

---

# Curriculum Complete!
Congratulations! You have completed all 7 stages of the Systems C and Unix internals curriculum. Use [02_README.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/02_README.md) to navigate and review any exercise or concept note.
