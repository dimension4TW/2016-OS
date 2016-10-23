#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define TOK_SIZE 1024
#define DELETE " \t\r\n\a"
#define FG_MODE 0
#define BG_MODE 1

using namespace std;

struct instruction{

    int status;
    int mode;
    bool inpt;
    bool outpt;
    char **args;
    char *iofile;
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
    if (pid == 0) {
        // Child process
        if(a.outpt){
            //cout<<"GG"<<endl;
            //cout<<a.iofile<<endl;
            int fd = open(a.iofile, O_RDWR|O_CREAT, 0644);
            close(1);
            dup2(fd,1);
            //cout<<"GG"<<endl;
        }

        
        if(a.inpt){
            int fd = open(a.iofile, O_RDONLY);
            close(0);
            dup2(fd,0);
        }
        

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

bool IO_redirection_check(char check, char* line){
    if(strchr(line, check) != NULL){
        return true;
    }
    else {
        return false;
    }
}

void mysh_loop(void) {
    char *line;
    char *re_line;
    instruction p;
    p.inpt = false;
    p.outpt = false;

    do {
        printf("> ");
        //p.iofile = (char*)malloc(sizeof(char)*BUFFER_SIZE);
        line = mysh_read_line();
        p.mode = mysh_check_mode(line);
        p.outpt = IO_redirection_check('>',line);
        p.inpt = IO_redirection_check('<',line);
        //cout<<"input: "<<p.inpt<<endl;
        //cout<<"output: "<<p.outpt<<endl;
        if(p.outpt){
            re_line = strtok(line,">");
            p.iofile = strtok(NULL," >");
            strcpy (line,re_line);
            cout<<"re_line: "<<line<<endl;
            cout<<"dirfile: "<<p.iofile<<endl;
        }
        else if(p.inpt){
            re_line = strtok(line,"<");
            p.iofile = strtok(NULL,"< ");
            cout<<"re_line: "<<re_line<<endl;
            cout<<"dirfile: "<<p.iofile<<endl;
        }
        //p.iofile = NULL;
        p.args = mysh_split_line(line);
        p.status = mysh_execute(p);

        free(line);
        free(p.args);
        //free(p.iofile);

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

