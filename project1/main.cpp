#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <signal.h>
#include <termcap.h>

#define BUFFER_SIZE 1024
#define TOK_SIZE 1024
#define DELETE " \t\r\n\a"
#define FG_MODE 0
#define BG_MODE 1

using namespace std;

struct instruction{

    int status;
    int mode;
    char **args;
};

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

int mysh_launch(instruction a)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    cout<<"["<<pid<<"]"<<endl;
    if (pid == 0) {
        // Child process
        if (execvp(a.args[0], a.args) == -1) {
            perror("myshell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("fork fail");
    } else {
        // Parent process
        if(a.mode==FG_MODE) {
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
        else if(a.mode==BG_MODE){
            if (kill(pid, SIGCONT) < 0) {
                perror("kill (SIGCONT)");
            }
            //cout<<"Done ["<<pid<<"]"<<endl;
        }
    }

    return 1;
}

int mysh_execute(instruction a)
{
    if (a.args[0] == NULL) {
        return 1;
    }

    return mysh_launch(a);
}

int mysh_check_mode(char* line){

    for(int i=(int)strlen(line)-1;i>=0;i--){
        if(line[i] == '&'){
            line[i] = ' ';
            return BG_MODE;
        }
        else if(line[i] != ' '){
            break;
        }
    }
    return FG_MODE;
}


void mysh_loop(void) {
    char *line;
    instruction p;

    do {
        printf("> ");
        line = mysh_read_line();
        p.mode = mysh_check_mode(line);
        //cout<<"Mode: "<<p.mode<<endl;
        p.args = mysh_split_line(line);
        p.status = mysh_execute(p);

        free(line);
        free(p.args);
    } while (p.status);
}

void signal_handler(int signal) {
    const char *signal_name;
    switch (signal) {
        /*
        case SIGINT:
            signal_name = "SIGINT";
            break;
        case SIGTSTP:
            signal_name = "SIGTSTP";
            break;

        */
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

int main(int argc, char **argv)
{


    /*
    if (signal(SIGINT, signal_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");

    if (signal(SIGTSTP, signal_handler) == SIG_ERR)
        printf("\ncan't catch SIGTSTP\n");
    */
    
    if (signal(SIGCHLD, signal_handler) == SIG_ERR)
        printf("\ncan't catch SIGCHLD\n");

    mysh_loop();

    return EXIT_SUCCESS;
}

