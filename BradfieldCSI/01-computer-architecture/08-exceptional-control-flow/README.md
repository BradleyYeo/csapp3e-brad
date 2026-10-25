# Exceptional Control Flow: Systems Programming and Unix Shell

## Mental Models and Theoretical Foundations

### Exceptional Control Flow (ECF) Taxonomy
- Modern computer systems achieve concurrency and handle asynchronous external events through Exceptional Control Flow:
  - **Interrupts**: Asynchronous hardware signals (e.g. keyboard keystroke, network packet arrival, hardware timer tick). Returns control to next instruction.
  - **Traps**: Synchronous intentional exceptions invoked by software instructions (e.g. `syscall` / system calls, software breakpoints). Returns to next instruction.
  - **Faults**: Synchronous unintentional error conditions that may be recoverable (e.g. page faults, floating-point divide-by-zero). Re-executes the faulting instruction if resolved by OS kernel, or terminates process.
  - **Aborts**: Synchronous unrecoverable hardware parity/bus errors. Immediately terminates the process.

### The Process Abstraction
- A process provides two key operating system illusions:
  - **Logical Control Flow**: The illusion that the process has exclusive use of the CPU, implemented via kernel context switching.
  - **Private Address Space**: The illusion that the process has exclusive access to the memory space, implemented via hardware page tables and the Memory Management Unit (MMU).

### Process Lifecycle and System Calls
- `fork()`: Creates a new child process with an exact duplicate of the parent's virtual address space, file descriptor table, and register state.
  - Returns $0$ in child process.
  - Returns child PID in parent process.
  - Returns $-1$ on error.
- `execve()` / `execvp()`: Loads and runs a new executable binary within the current process context. Never returns on success; only returns $-1$ on error.
- `waitpid()`: Reaps terminated child processes to prevent zombie processes.
- `exit()`: Terminates process execution and reports integer exit status.

### Unix Signals and Asynchronous Signal Safety
- A signal is a short message sent by the kernel to a process notifying it that an event has occurred (e.g. `SIGINT`, `SIGTSTP`, `SIGCHLD`).
- Signal handlers run concurrently with main application logic.
- **Async-Signal-Safe Invariant**: Signal handlers must only invoke reentrant, async-signal-safe system functions (such as `write()`). Calling non-reentrant functions like `printf()` or `malloc()` from inside a signal handler can corrupt heap metadata and cause deadlocks.
- **Zombie Process Prevention**: When a child terminates, it remains in a "zombie" state until reaped by its parent via `waitpid()`. If multiple children terminate while the `SIGCHLD` handler is running, signals will be coalesced (Unix signals are not queued!). Therefore, the `SIGCHLD` handler must loop with `waitpid(-1, &status, WNOHANG)` to reap all pending terminated children.

## Textbook Reference
- **CS:APP3e**: Chapter 8 (*Exceptional Control Flow*), Sections 8.1–8.5.

## Problem Set and Specifications

### Exercise: Unix Shell Implementation (`shell.c`)
- Objective: Build a functional Unix command shell from scratch in C.
- Files:
  - [shell.c](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/08-exceptional-control-flow/shell.c): Starter template.
  - [shell.h](file:///Users/bradleyyeo/Documents/learn/csapp3e-brad/BradfieldCSI/01-computer-architecture/08-exceptional-control-flow/shell.h): Header declarations.

### Step-by-Step Implementation Guide
- **Step 1: Process Creation and Program Execution (`eval`)**
  - Use `fork()` to create child process.
  - In child: invoke `execvp(argv[0], argv)`. If command not found, print error and call `exit(1)`.
- **Step 2: Foreground vs. Background Execution**
  - Check trailing `&` parameter.
  - If foreground: parent calls `waitpid(pid, &status, 0)` and blocks until child exits.
  - If background: parent prints background job PID and immediately prompts for next command.
- **Step 3: Built-in Commands (`builtin_command`)**
  - Support `exit` / `quit`: terminates shell.
  - Support `cd <path>`: uses `chdir()` system call.
- **Step 4: Signal Forwarding (`SIGINT` and `SIGTSTP`)**
  - Install handlers using `signal(SIGINT, sigint_handler)` and `signal(SIGTSTP, sigtstp_handler)`.
  - In child process, set separate process group `setpgid(0, 0)` so Ctrl+C / Ctrl+Z does not kill the parent shell.
  - When signal received in parent, forward to active foreground child process group using `kill(-fg_pid, sig)`.
- **Step 5: Asynchronous Child Reaping (`SIGCHLD`)**
  - Install handler `signal(SIGCHLD, sigchld_handler)`.
  - Inside handler, reap all dead children:
    ```c
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        // Child reaped cleanly without blocking
    }
    ```

## Build and Verification
- Compile shell:
  ```bash
  gcc -Wall -Wextra -g shell.c -o shell
  ```
- Run and test interactively:
  ```bash
  ./shell
  ```
  - Test external commands: `ls -la`, `echo hello`, `sleep 2`
  - Test background jobs: `sleep 5 &`
  - Test Ctrl+C (`SIGINT`): start `sleep 10` and press Ctrl+C; verify only the child terminates while the shell prompt returns.

## Active Recall Checkpoint
- What is a zombie process, what system resources does it consume, and how does `waitpid()` reap it?
- Why are Unix signals not queued, and how does this affect writing a correct `SIGCHLD` handler?
- Why must `chdir()` be implemented as an internal shell builtin rather than an external program like `/bin/ls`?
