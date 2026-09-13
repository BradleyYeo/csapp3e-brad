# Stage 7: Exercise 15 - Process Lifecycle, Pipelines, and Program Execution

A guide explaining Unix process creation (`fork`), exit codes, zombie reaping (`waitpid`), signal dispatch (`kill`), pipes (`pipe`), descriptor redirection (`dup2`), and program execution (`execvp`) for C beginners.

# Conceptual Foundations and Mental Model

## Process Creation via fork()
- In Unix, new processes are created by cloning an existing process using `fork()`.
- The kernel clones the calling process: memory mappings, file descriptor table, environment variables, and instruction pointer.
- Copy-on-Write (COW): Physical memory pages are initially shared read-only between parent and child. If either process attempts to write to a page, the CPU triggers a page fault, and the kernel creates a private copy of that specific 4KB page.
- Return values of `fork()`:
  - In the child process: Returns `0`.
  - In the parent process: Returns the child's positive Process ID (`pid > 0`).
  - On failure: Returns `-1`.

## Zombies and Reaping with waitpid()
- When a child process calls `exit(status)`, its memory and open files are immediately freed, but its entry in the kernel's Process Table remains allocated (a "Zombie" process).
- The zombie retains the child's PID and exit status so the parent can inspect how it finished.
- Reaping: The parent calls `waitpid(pid, &status, 0)`:
  - Suspends the parent until the child terminates.
  - Reaps the zombie entry from the kernel table.
  - Inspecting status:
    - `WIFEXITED(status)`: Evaluates true if child called `exit()` normally.
    - `WEXITSTATUS(status)`: Extracts the integer exit status (0-255).
    - `WIFSIGNALED(status)`: Evaluates true if child was terminated by an unhandled signal (e.g. `SIGKILL`, `SIGSEGV`).
    - `WTERMSIG(status)`: Extracts the terminating signal number.

## Redirection and Exec: dup2() and execvp()
- Descriptors 0, 1, and 2 are standard:
  - `0`: `STDIN_FILENO` (Standard Input)
  - `1`: `STDOUT_FILENO` (Standard Output)
  - `2`: `STDERR_FILENO` (Standard Error)
- System call `dup2(oldfd, newfd)`:
  - Closes `newfd` (if open).
  - Copies descriptor table entry `oldfd` into slot `newfd`.
  - Calling `dup2(pipefd[1], STDOUT_FILENO)` causes any standard output (`printf`, `write(1)`) to flow directly into the pipe!
- System call `execvp(file, argv)`:
  - Replaces the current process image entirely with the executable binary specified by `file`.
  - The stack, heap, and code segments are wiped clean and overwritten by the new program.
  - Open file descriptors survive across `execvp` (unless marked `FD_CLOEXEC`).
  - Notice: `execvp` NEVER returns upon success! It only returns if an error occurred (e.g. command not found).

## The Pipe EOF Invariant (Closing Unused Ends)
- A pipe has two ends: `pipefd[0]` (read end) and `pipefd[1]` (write end).
- Reading from `pipefd[0]` returns `0` (EOF) ONLY when ALL write descriptors across ALL processes pointing to that pipe are closed.
- Invariant: If a child or parent forgets to close its inherited copy of `pipefd[1]`, a reading process will hang indefinitely waiting for more data, causing a deadlock!

---

# Line-by-Line Code Breakdown

## Function: test_fork_and_wait

```c
static bool test_fork_and_wait(int expected_exit_code) {
  pid_t parent_pid = getpid();
  pid_t pid = fork();

  if (pid < 0) {
    return false;
  }

  if (pid == 0) {
    // Child process execution path
    exit(expected_exit_code);
  }

  // Parent process execution path
  int status = 0;
  pid_t reaped_pid = waitpid(pid, &status, 0);
  if (reaped_pid != pid) {
    return false;
  }

  if (!WIFEXITED(status) || WEXITSTATUS(status) != expected_exit_code) {
    return false;
  }

  return true;
}
```

### Process Branching
- `if (pid == 0)`: Code executed strictly by the child process.
- `exit(expected_exit_code)`: Terminates child and notifies kernel.
- Parent calls `waitpid(pid, &status, 0)`, blocking until child exits.
- `WIFEXITED(status)` confirms clean termination without crashes.

---

## Function: test_pipeline_exec (Echo piped to Cat)

```c
static bool test_pipeline_exec(void) {
  int pfd[2];
  if (pipe(pfd) != 0) {
    return false;
  }

  pid_t pid1 = fork();
  if (pid1 == 0) {
    // Child 1: echo "systems_pipeline_ok"
    close(pfd[0]); // Close unused read end
    dup2(pfd[1], STDOUT_FILENO); // Redirect stdout to pipe write end
    close(pfd[1]); // Close duplicate descriptor

    char *argv[] = {"echo", "systems_pipeline_ok", nullptr};
    execvp(argv[0], argv);
    _exit(EXIT_FAILURE); // Only reached on exec failure
  }

  pid_t pid2 = fork();
  if (pid2 == 0) {
    // Child 2: cat (reads from pipe stdin)
    close(pfd[1]); // Close unused write end (Crucial for EOF!)
    dup2(pfd[0], STDIN_FILENO); // Redirect stdin to pipe read end
    close(pfd[0]); // Close duplicate descriptor

    char *argv[] = {"cat", nullptr};
    execvp(argv[0], argv);
    _exit(EXIT_FAILURE);
  }

  // Parent closes both pipe ends so it doesn't hold open descriptors
  close(pfd[0]);
  close(pfd[1]);

  // Wait for both children
  int s1 = 0, s2 = 0;
  waitpid(pid1, &s1, 0);
  waitpid(pid2, &s2, 0);

  return (WIFEXITED(s1) && WEXITSTATUS(s1) == 0 &&
          WIFEXITED(s2) && WEXITSTATUS(s2) == 0);
}
```

### Pipeline Flow
- `pipe(pfd)`: Allocates kernel ring buffer. `pfd[0]` = read, `pfd[1]` = write.
- Child 1 replaces stdout with `pfd[1]` and executes `echo`.
- Child 2 replaces stdin with `pfd[0]` and executes `cat`.
- Crucial invariant: Child 2 and Parent BOTH close `pfd[1]`. When Child 1 finishes and exits, all write descriptors to the pipe close. Child 2's `cat` receives EOF and terminates cleanly.

---

# Memory Layout Visualization

Process Cloning via `fork()`:

```
Before fork():
  Parent Process (PID 100):
  Virtual Space: [ Code (r-x) ][ Data (rw-) ][ Stack ][ Heap ]
  Descriptor Table: [ 0: stdin ][ 1: stdout ][ 2: stderr ]

After fork():
  Parent Process (PID 100):                       Child Process (PID 101):
  Returns: pid = 101                             Returns: pid = 0
  Virtual Space: [ Shared COW Pages ]            Virtual Space: [ Shared COW Pages ]
  Descriptor Table: [ 0 ][ 1 ][ 2 ]              Descriptor Table: [ 0 ][ 1 ][ 2 ]
```

Inter-Process Pipeline Architecture:

```
Child 1 (PID 101: echo):                         Child 2 (PID 102: cat):
  Descriptor Table:                                Descriptor Table:
    [ 0: stdin  ]                                    [ 0: STDIN_FILENO ] <---+
    [ 1: STDOUT_FILENO ] ---+                        [ 1: stdout       ]     |
                            |                                                |
                            v                                                |
                 +--------------------------------------+                    |
                 | Kernel Pipe Ring Buffer (Capacity 64KB) |                    |
                 | [ "systems_pipeline_ok\n" ]          |                    |
                 +--------------------------------------+ -------------------+
```

---

# Active Recall and Validation Drills

## Drill: Active Recall Questions
- Question 1: What is a zombie process, and how does calling `waitpid()` prevent memory leaks in the kernel Process Table?
- Question 2: Why must `cat`'s process close the write end of the pipe (`pfd[1]`) before executing? What happens if it forgets?
- Question 3: Why does code placed after `execvp()` only execute if `execvp()` fails?

## Drill: Hands-On Code Validation
- Open [15_process_pipeline_exec.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/15_process_pipeline_exec.c).
- Implement a 3-stage pipeline: `echo "banana\napple\ncherry" | sort | wc -l`.
- Verify using `waitpid` that all 3 child processes terminate with exit code 0.
- Compile and run:
  ```bash
  clang -std=c23 -Wall -Wextra -Werror -pedantic -g -o 15_process_pipeline_exec 15_process_pipeline_exec.c
  ./15_process_pipeline_exec
  ```

---

# Next Step in Curriculum
Proceed to [12_pipe_pingpong.md](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/exercises/12_pipe_pingpong.md) to benchmark inter-process communication latency with bi-directional pipe ping-pong.
