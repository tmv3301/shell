#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 100

void execute_command(char *command);
void parse_command(char *command, char **args);
void change_directory(char *path);
void print_working_directory();
void handle_exit();
void handle_pipes(char *command);
void run_in_background(char *command);
void remove_newline(char *str);
void wait_for_child();
void handle_echo(char *command);
void handle_ls(char *command);
void handle_command_substitution(char *command);
void handle_multiple_commands(char *command);

int main() {
    char command[MAX_CMD_LEN];

    while (1) {
        char cwd[MAX_CMD_LEN];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s$ ", cwd);
        }
		else {
            perror("getcwd failed");
        }

        if (fgets(command, MAX_CMD_LEN, stdin) == NULL) {
            break;
        }

        remove_newline(command);

        handle_command_substitution(command);

        if (strcmp(command, "exit") == 0) {
            handle_exit();
        }
		else if (strstr(command, "cd ") == command) {
            change_directory(command + 3);
        }
		else if (strcmp(command, "pwd") == 0) {
            print_working_directory();
        }
		else if (strncmp(command, "echo", 4) == 0) {
            handle_echo(command);
        }
		else if (strncmp(command, "ls", 2) == 0) {
            handle_ls(command);
        }
		else if (strchr(command, '|')) {
            handle_pipes(command);
        }
		else if (strchr(command, '&')) {
            run_in_background(command);
        }
		else if (strchr(command, ';') || strchr(command, '&') || strchr(command, '||') || strchr(command, '&&')) {
            handle_multiple_commands(command);
        }
		else {
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
    }
	else if (pid < 0) {
        perror("Fork failed");
    }
	else {
        wait_for_child();
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
    }
	else {
        perror("pwd failed");
    }
}

void handle_exit() {
    printf("Exiting shell...\n");
    exit(0);
}

void remove_newline(char *str) {
    str[strcspn(str, "\n")] = 0;
}

void handle_echo(char *command) {
    char *args[MAX_ARGS];
    parse_command(command, args);
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
}

void handle_ls(char *command) {
    char *args[MAX_ARGS];
    parse_command(command, args);
    pid_t pid = fork();
    if (pid == 0) {
        execvp("ls", args);
        exit(1);
    } else {
        wait_for_child();
    }
}

void handle_command_substitution(char *command) {
    char *start, *end, temp[MAX_CMD_LEN];
    char result[MAX_CMD_LEN];

    while ((start = strchr(command, '$')) != NULL) {
        if (*(start + 1) == '(') {
            end = strchr(start + 2, ')');
            if (end != NULL) {
                strncpy(temp, start + 2, end - start - 2);
                temp[end - start - 2] = '\0';


                FILE *fp = popen(temp, "r");
                if (fp != NULL) {
                    fgets(result, sizeof(result), fp);
                    fclose(fp);
                    *start = '\0';
                    strcat(command, result);
                    strcat(command, end + 1);
                }
            }
        }
    }
}

void handle_pipes(char *command) {
    int pipe_fd[2];
    pid_t pid1, pid2;
    pipe(pipe_fd);
    pid1 = fork();
    if (pid1 == 0) {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);

        char *args[MAX_ARGS];
        parse_command(command, args);
        execvp(args[0], args);
        exit(1);
    }

    pid2 = fork();
    if (pid2 == 0) {
        close(pipe_fd[1]);
        dup2(pipe_fd[0], STDIN_FILENO);
        close(pipe_fd[0]);

        char *args[MAX_ARGS];
        parse_command(command, args);
        execvp(args[0], args);
        exit(1);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void run_in_background(char *command) {
    pid_t pid = fork();
    if (pid == 0) {
        char *args[MAX_ARGS];
        parse_command(command, args);
        execvp(args[0], args);
        exit(1);
    }
}

void wait_for_child() {
    pid_t pid = wait(NULL);
    if (pid == -1) {
        perror("Wait failed");
    }
}

void handle_multiple_commands(char *command) {
    char *token = strtok(command, ";");
    while (token != NULL) {
        execute_command(token);
        token = strtok(NULL, ";");
    }
}
