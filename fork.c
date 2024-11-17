#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 1024
#define MAX_ARG_LENGTH 100
#define MAX_PROCESSES 100 // Maximum number of processes to track

pid_t process_stack[MAX_PROCESSES]; // Stack to store PIDs
int stack_top = -1; // Stack pointer

// Signal handler for SIGINT
void handle_sigint(int sig) {
    while (stack_top >= 0) {
        pid_t pid = process_stack[stack_top--]; // Pop from stack
        kill(pid, SIGTERM); // Terminate the child process
        printf("Terminated process with PID %d\n", pid);
    }
}

void execute_command(char *command) {
    char *args[MAX_ARG_LENGTH];
    char *token = strtok(command, " \n");
    int i = 0;

    while (token != NULL && i < MAX_ARG_LENGTH - 1) {
        args[i++] = token;
        token = strtok(NULL, " \n");
    }
    args[i] = NULL; // Null-terminate the argument list

    // Fork a child process
    pid_t child_pid = fork();
    if (child_pid < 0) {
        perror("Fork failed");
        return;
    }

    if (child_pid == 0) {
        // Child process
        if (strcmp(args[0], "ls") == 0) {
            execvp("ls", args);
        } else if (strcmp(args[0], "cat") == 0) {
            execvp("cat", args);
        } else if (strcmp(args[0], "nice") == 0) {
            execvp("nice", args);
        } else if (strcmp(args[0], "killall") == 0) {
            execvp("killall", args);
        } else {
            // Execute any other program
            execvp(args[0], args);
            perror("Command execution failed");
        }
        exit(EXIT_FAILURE); // Exit if exec fails
    } else {
        // Parent process
        if (stack_top < MAX_PROCESSES - 1) {
            process_stack[++stack_top] = child_pid; // Push PID onto stack
        } else {
            fprintf(stderr, "Process stack overflow\n");
        }
        waitpid(child_pid, NULL, 0); // Wait for the child process to finish
    }
}

int main() {
    char input[MAX_INPUT_LENGTH];

    // Set up the signal handler for SIGINT
    signal(SIGINT, handle_sigint);

    while (1) {
        printf("my_shell> "); // Custom shell prompt
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; // Exit on EOF
        }
        // If input is empty, skip the iteration
        if (strlen(input) == 0) {
            continue;
        }
        execute_command(input);
    }

    return 0;
}
