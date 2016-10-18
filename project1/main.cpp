#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <termcap.h>

#define BUFFER_SIZE 1024
#define TOK_SIZE 64
#define DELETE " \t\r\n\a"

using namespace std;

char *mysh_read_line(void)
{

    char* line;
    line = (char*)malloc(sizeof(char)*BUFFER_SIZE);
    cin.getline(line,BUFFER_SIZE);

    return line;
}

char **mysh_split_line(char *line)
{
    int bufsize = TOK_SIZE, position = 0;
    char **tokens = (char**)malloc(bufsize * sizeof(char*));
    char *token;

    token = strtok(line,DELETE);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        token = strtok(NULL,DELETE);
    }
    tokens[position] = NULL;
    return tokens;
}

int mysh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("lsh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int mysh_execute(char **args)
{
    if (args[0] == NULL) {
        return 1;
    }

    return mysh_launch(args);
}

void mysh_loop(void) {
    char *line;
    char **args;
    int check;

    do {
        printf("> ");
        line = mysh_read_line();
        args = mysh_split_line(line);
        check = mysh_execute(args);

        free(line);
        free(args);
    } while (check);
}

int main(int argc, char **argv)
{

    mysh_loop();

    return EXIT_SUCCESS;
}

