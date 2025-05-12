#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 100

void execute_command(char *command);
void parse_command(char *command, char **args);
void change_directory(char *path);
void print_working_directory();
void handle_exit();

int main() {
    char command[MAX_CMD_LEN];

    while (1) {
        printf("my_shell> ");
        if (fgets(command, MAX_CMD_LEN, stdin) == NULL) {
            break;
        }

        command[strcspn(command, "\n")] = 0;

        if (strcmp(command, "exit") == 0) {
            handle_exit();
        } else if (strstr(command, "cd ") == command) {
            change_directory(command + 3);
        } else if (strcmp(command, "pwd") == 0) {
            print_working_directory();
        } else {
            execute_command(command);
        }
    }
    return 0;
}

void execute_command(char *command) {
    char *args[MAX_ARGS];
    pid_t pid;
    int status;

    parse_command(command, args);

    pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1) {
            perror("Execution failed");
        }
        exit(1);
    } else if (pid < 0) {
        perror("Fork failed");
    } else {
        waitpid(pid, &status, 0);
    }
}

void parse_command(char *command, char **args) {
    int i = 0;
    char *token = strtok(command, " ");

    while (token != NULL) {
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

void change_directory(char *path) {
    if (chdir(path) != 0) {
        perror("cd failed");
    }
}

void print_working_directory() {
    char cwd[MAX_CMD_LEN];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd failed");
    }
}

void handle_exit() {
    printf("Exiting shell...\n");
    exit(0);
}


