#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_ARGS 128
#define PROMPT "csapp-shell> "

/*
 * CS:APP Chapter 8: Exceptional Control Flow
 * Building a Unix Shell from Scratch
 *
 * Concepts to Master:
 * 1. Process Lifecycle: fork(), execve() / execvp(), waitpid(), exit()
 * 2. Foreground vs. Background execution (trailing '&')
 * 3. Builtin Commands: quit, exit, cd, jobs
 * 4. Signal Handling:
 *    - SIGINT (Ctrl+C): forward to foreground process group
 *    - SIGTSTP (Ctrl+Z): forward to foreground process group
 *    - SIGCHLD: asynchronously reap terminated/stopped child processes without race conditions
 */

// Global variable tracking the PID of current foreground child
static volatile pid_t fg_pid = 0;

// Function Prototypes
void eval(char *cmdline);
int parse_line(char *buf, char **argv);
int builtin_command(char **argv);
void sigint_handler(int sig);
void sigtstp_handler(int sig);
void sigchld_handler(int sig);

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    (void)fg_pid;
    char cmdline[MAX_LINE];

    // TODO: Step 4 - Install Signal Handlers
    // signal(SIGINT, sigint_handler);
    // signal(SIGTSTP, sigtstp_handler);
    // signal(SIGCHLD, sigchld_handler);

    while (1) {
        printf("%s", PROMPT);
        fflush(stdout);

        if ((fgets(cmdline, MAX_LINE, stdin) == NULL) && ferror(stdin)) {
            perror("fgets error");
            exit(1);
        }

        if (feof(stdin)) { // End of file (Ctrl+D)
            printf("\nExiting shell.\n");
            exit(0);
        }

        // Evaluate and execute command
        eval(cmdline);
    }

    return 0;
}

/*
 * eval: Evaluate and run a command line.
 * If command is builtin, execute immediately.
 * Otherwise, fork child process and execvp.
 */
void eval(char *cmdline) {
    char *argv[MAX_ARGS];
    char buf[MAX_LINE];
    int bg;

    strcpy(buf, cmdline);
    bg = parse_line(buf, argv);

    if (argv[0] == NULL) {
        return; // Ignore empty lines
    }

    if (!builtin_command(argv)) {
        // TODO: Step 1 - Fork child process
        // pid = fork();
        // if (pid < 0) { perror("fork error"); return; }
        //
        // if (pid == 0) { // Child process
        //     // TODO: Set process group ID so signals only target foreground job
        //     // setpgid(0, 0);
        //     // if (execvp(argv[0], argv) < 0) {
        //     //     printf("%s: Command not found.\n", argv[0]);
        //     //     exit(1);
        //     // }
        // }
        //
        // TODO: Step 2 - Parent waits for foreground job, or prints background job status
        // if (!bg) {
        //     fg_pid = pid;
        //     // wait for foreground child to complete
        //     // int status;
        //     // waitpid(pid, &status, 0);
        //     // fg_pid = 0;
        // } else {
        //     printf("[%d] %s\n", pid, cmdline);
        // }

        printf("[eval TODO] Executing external command: %s (bg=%d)\n", argv[0], bg);
    }
}

/*
 * builtin_command: If first arg is a builtin command, run it and return true.
 */
int builtin_command(char **argv) {
    if (strcmp(argv[0], "quit") == 0 || strcmp(argv[0], "exit") == 0) {
        exit(0);
    }
    if (strcmp(argv[0], "cd") == 0) {
        // TODO: Step 3 - Implement cd builtin
        // if (argv[1] == NULL) { chdir(getenv("HOME")); }
        // else { if (chdir(argv[1]) != 0) perror("cd failed"); }
        return 1;
    }
    if (strcmp(argv[0], "&") == 0) {
        return 1; // Ignore singleton &
    }
    return 0; // Not a builtin
}

/*
 * parse_line: Parse command line into argv array.
 * Returns 1 if job should run in background, 0 if foreground.
 */
int parse_line(char *buf, char **argv) {
    char *delim;
    int argc = 0;
    int bg;

    buf[strlen(buf) - 1] = ' '; // Replace trailing '\n' with space
    while (*buf && (*buf == ' ')) buf++; // Ignore leading spaces

    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) buf++;
    }
    argv[argc] = NULL;

    if (argc == 0) return 1;

    // Check if background job requested with '&'
    if ((bg = (*argv[argc - 1] == '&')) != 0) {
        argv[--argc] = NULL;
    }

    return bg;
}

/*
 * sigint_handler: Catch SIGINT (Ctrl+C).
 * Forward to active foreground process group.
 */
void sigint_handler(int sig) {
    (void)sig;
    // TODO: Forward SIGINT to fg_pid using kill(-fg_pid, SIGINT)
    // Be careful to only use async-signal-safe functions like write()
}

/*
 * sigtstp_handler: Catch SIGTSTP (Ctrl+Z).
 * Forward to active foreground process group.
 */
void sigtstp_handler(int sig) {
    (void)sig;
    // TODO: Forward SIGTSTP to fg_pid using kill(-fg_pid, SIGTSTP)
}

/*
 * sigchld_handler: Asynchronously reap child processes.
 * Must loop with WNOHANG to reap all available dead children without blocking.
 */
void sigchld_handler(int sig) {
    (void)sig;
    // TODO: Reap terminated children in a loop:
    // int status;
    // pid_t pid;
    // while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
    //     // Clean up terminated or stopped child
    // }
}
