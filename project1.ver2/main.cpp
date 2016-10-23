#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <iostream>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define PIPE_BUFFER 1024
#define TOK_SIZE 1024
#define DELETE " \t\r\n\a"

#define FG_MODE 0
#define BG_MODE 1


#define NONE "\033[m"
#define LIGHT_PURPLE "\033[1;35m"

using namespace std;


struct command_segment {
    char **args;   // arguments array
    struct command_segment *next;
    pid_t pid;   // process ID
    pid_t pgid;   // process group ID
};

struct command {
    struct command_segment *root;   // a linked list
    int mode;   // BACKGROUND_EXECUTION or FOREGROUND_EXECUTION
};


int mysh_bg(pid_t pid) {
    //printf(LIGHT_PURPLE"bg gpid: %d\n"NONE, pid);
    if (kill(pid, SIGCONT) < 0) {
        perror("kill (SIGCONT)");
    }
}

int mysh_execute_command_segment(struct command_segment *segment, int in_fd, int out_fd, int mode, int pgid,
                                 int counter, int total) {
    int pid, status, exec_status;

    pid = fork();
    if (pid == 0) {
        if (counter == 0 && counter != total - 1) {
            close(in_fd);
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        else if (counter != total - 1) {
            dup2(in_fd, STDIN_FILENO);
            dup2(out_fd, STDOUT_FILENO);
            close(in_fd);
            close(out_fd);
        }
        else {
            close(out_fd);
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }

        exec_status = execvp(segment->args[0], segment->args);
        if (exec_status == -1) {
            printf("command not found\n");
            exit(0);
        }
    }
    else if (pid > 0) {
        //fprintf(stderr, LIGHT_PURPLE"Command executed by pid=%d\n"NONE, pid);
        close(in_fd);
        close(out_fd);
        segment->pid = pid;
        if (pgid == 0) {
            segment->pgid = pid;
        }
        else {
            segment->pgid = pgid;
        }
        setpgid(pid, segment->pgid);
        if (mode == BG_MODE) {
            mysh_bg(pid);
        }
    }
    else {
        printf("fork error QQ\n");
    }
}

int mysh_execute_command(struct command *command) {
    struct command_segment *cur_segment;
    int pipe_fd[PIPE_BUFFER][2];
    int p_gid = 0;
    int counter = 0;
    int total = 0;
    int status = 0;
    for (cur_segment = command->root; cur_segment != NULL; cur_segment = cur_segment->next) {
        total++;
    }

    pipe(pipe_fd[0]);
    close(pipe_fd[0][1]);
    for (cur_segment = command->root; cur_segment != NULL; cur_segment = cur_segment->next) {
        if (pipe(pipe_fd[counter + 1]) == -1) {
            printf("pipe error");
        }
        status = mysh_execute_command_segment(cur_segment, pipe_fd[counter][0], pipe_fd[counter + 1][1], command->mode,
                                              p_gid, counter, total);
        p_gid = cur_segment->pgid;
        if (counter == 0) {
            tcsetpgrp(0, cur_segment->pid);
        }
        counter++;
    }
    close(pipe_fd[counter][0]);
    for (int i = 0; i < counter && command->mode == FG_MODE; ++i) {
        waitpid(-command->root->pgid, &status, WUNTRACED);
    }
    tcsetpgrp(0, getpid());
    return 0;
}

struct command_segment *mysh_parse_command_segment(char *segment) {

    struct command_segment *c = (struct command_segment *) malloc(sizeof(struct command_segment));
    int bufsize = TOK_SIZE, position = 0;
    char **tokens = (char**)malloc(bufsize * sizeof(char*));
    char *token;
    token = strtok(segment,DELETE);
    while (token != NULL) {
        tokens[position] = token;
        position++;
        token = strtok(NULL,DELETE);
    }
    tokens[position] = NULL;
    c->args = tokens;
    c->next = NULL;
    return c;
}

struct command *mysh_parse_command(char *line) {
    int mode = FG_MODE;
    for (int i = (int)strlen(line)-1;i >= 0;i--) {
        if (line[i] == '&') {
            mode = BG_MODE;
            line[i] = ' ';
        }
        if (line[i] != ' ') {
            break;
        }
    }

    struct command *cmd = (struct command *) malloc(sizeof(struct command));
    char *pipeline = strtok(line, "|");
    char **pipeline_args = (char **) malloc(BUFFER_SIZE * sizeof(char *));
    int pipeline_counter = 0;
    while (pipeline != NULL) {
        pipeline_args[pipeline_counter++] = pipeline;
        pipeline = strtok(NULL, "|");
    }
    cmd->root = mysh_parse_command_segment(pipeline_args[0]);
    struct command_segment *next_command = cmd->root;
    for (int i = 1; i < pipeline_counter; i++) {
        next_command->next = mysh_parse_command_segment(pipeline_args[i]);
        next_command = next_command->next;
    }
    cmd->mode = mode;
    return cmd;
}

char *mysh_read_line() {
    char* line;
    line = (char*)malloc(sizeof(char)*BUFFER_SIZE);
    cin.getline(line,BUFFER_SIZE);
    return line;
}

void mysh_loop() {
    char *line;
    struct command *command;
    int status = 1;
    do {
        printf("> ");
        line = mysh_read_line();   // read one line from terminal
        if (strlen(line) == 0) {
            continue;
        }
        command = mysh_parse_command(line);   // parse line as command structure
        status = mysh_execute_command(command);   // execute the command
        free(line);
    } while (status >= 0);
}


void handle_signal(int signal) {
    const char *signal_name;
    switch (signal) {

        case SIGCHLD:
            signal_name = "SIGCHLD";
            int pid, status;
            while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            }
            break;
        default:
            fprintf(stderr, "Error signal: %d\n", signal);
            return;
    }
}


void mysh_init() {

    struct sigaction sa;
    sa.sa_handler = &handle_signal;
    sa.sa_flags = SA_RESTART;
    sigfillset(&sa.sa_mask);
    struct sigaction sa_ign;
    sa_ign.sa_handler = SIG_IGN;
    sa_ign.sa_flags = SA_RESETHAND;
    sigfillset(&sa_ign.sa_mask);

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Error: cannot handle SIGCHLD");
    }

    if (sigaction(SIGTTOU, &sa_ign, NULL) == -1) {
        perror("Error: cannot handle SIGTTOU");
    }

    if (sigaction(SIGTTIN, &sa_ign, NULL) == -1) {
        perror("Error: cannot handle SIGTTIN");
    }

}

int main(int argc, char **argv) {
    mysh_init();
    mysh_loop();
    return EXIT_SUCCESS;
}