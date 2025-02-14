//##################################################
//
// file: main.c
// @Author:   Noriana Tzatzai csd5016
//
//###################################################

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>

#define TRUE 1

// sinartisi gia dimioyrgia pipes()
void pipes_command(char* command){
    char* commands[100];
    char* token=strtok(command, "|");
    int sum_cwd=0;
    int status, i=0;
    pid_t pid;
    while(token != NULL){
        commands[sum_cwd] = token;
        token = strtok(NULL, "|");
        sum_cwd++;
    }
    int pipefds[2 * (sum_cwd - 1)]; 
    while(i < sum_cwd - 1){
        if(pipe(pipefds + i * 2) == -1){
            printf("ERROR pipe\n");
            exit(1);
        }
        i++;
    }
    i=0;
    while(i < sum_cwd){
        pid=fork();
        if (pid == 0) {
            if (i > 0) {
                if (dup2(pipefds[(i - 1) * 2], STDIN_FILENO) == -1) {
                    perror("dup2");
                    exit(1);
                }
            }
            if (i < sum_cwd - 1) {
                if (dup2(pipefds[i * 2 + 1], STDOUT_FILENO) == -1) {
                    perror("dup2");
                    exit(1);
                }
            }
            int j=0;
            while(j < 2 * (sum_cwd - 1)){
                close(pipefds[j]);
                j++;
            }
            char* args[100];
            char* arg_token=strtok(commands[i], " \n");
            int arg_index=0;
            while (arg_token != NULL) {
                args[arg_index++] = arg_token;
                arg_token = strtok(NULL, " \n");
            }
            args[arg_index] = NULL;
            if(execvp(args[0], args) == -1){
                perror("execvp");
                exit(1);
            }
        }
        i++;
    }
    i=0;
    while(i < 2 * (sum_cwd - 1)){
        close(pipefds[i]);
        i++;
    }
    i=0;
    while(i < sum_cwd){
        wait(&status);
        i++;
    }
}

// sinartisi gia tin anakatefthinsi edsodou
void output_redirection(char* out_){
    //elegxw gia >>
    char* append_check=strstr(out_, ">>");
    char* command;
    char* filename;
    //kitaw an prepei na apanograpsei ta xwrizw me to >>
    if(append_check != NULL){
        command=strtok(out_, ">>");
        filename=strtok(NULL, ">>");
    }else{
        //alliw xvrizw me >
        command=strtok(out_, ">");
        filename=strtok(NULL, ">");
    }
    if(filename != NULL){
        while(*filename == ' '){// aferw kena 
            filename++;
        }
        int fd;
        if(append_check != NULL){
            fd=open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
        }else{
            fd=open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644);
        }
        if(fd < 0){
            perror("Error open file\n");
            exit(1);
        }
        dup2(fd, STDOUT_FILENO);//lew h eksodos na paei allou
        close(fd);
        char* args[100];
        char* token = strtok(command, " \n");
        int i=0;
        while(token != NULL){
            args[i++] = token;
            token = strtok(NULL, " \n");
        }
        args[i]=NULL;
        if(execvp(args[0], args) == -1){
            perror("Error execvp()\n");
            exit(1);
        }
    }else{
        printf("Error\n");
    }
}

// synartisi gia tin anakatefthinsi eisodoy
void input_redirection(char* in_){
    char* args[100];
    char* token;
    int fd,i;
    char* command=strtok(in_, "<");// prin to <
    char* filename=strtok(NULL, "<");// kai meta 
    if(filename != NULL){
        while(*filename==' ') filename++;
        fd=open(filename, O_RDONLY);
        if(fd < 0){
            perror("Error opening file");
            exit(1);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
        token=strtok(command, " \n");
        i=0;
        while(token!=NULL){
            args[i++]=token;
            token=strtok(NULL, " \n");
        }
        args[i]=NULL;
        if(execvp(args[0],args)==-1){
            perror("Error execvp()\n");
            exit(1);
        }
    } else {
        printf("Error\n");
    }
}

// sinartisi gia edoles ekxorisis x=10;
void global_variables_set(char* command){
    char* name=strtok(command, "=");
    char* value=strtok(NULL, "=");
    if(name!=NULL && value!= NULL){
        int tmp=setenv(name,value,1);
        if(tmp!=0){
            printf("Error setenv().\n");
            exit(0);
        }
    }
}

// sinartisi gia ton edopismo toy echo kai ektelesi toy
void global_variables_get(char *command) {
    char *token = strtok(command, " ");
    int first_token = 1;
    while(token != NULL){
        if(first_token && strcmp(token, "echo") == 0){
            first_token = 0;
        }else{
            char *dollar=strchr(token, '$');// kitaw gia $
            if(dollar != NULL){// tiponw opws einai prin to $
                while(token != dollar){
                    printf("%c", *token);
                    token++;
                }
                dollar++;// h global metavliti 
                char *value=getenv(dollar);
                if(value != NULL){
                    printf("%s", value);
                }else{
                    printf("");// an den uparxei den tiponw 
                }
            }else{
                printf("%s", token);
            }
        }
        printf(" ");
        token=strtok(NULL, " ");
    }
    printf("\n");
}


// sinartisi diaxeirisis command kai split me ";"
void _command(char* command){
    char* token;
    int k, i=0;
    char* str=strchr(command, '=');
    char* tmp=strstr(command, "echo");
    char* in=strchr(command, '<');
    char* out=strchr(command, '>');
    char* pip=strchr(command, '|');
    if(str != NULL){
        global_variables_set(command);  // set environment variables
    }else if(tmp != NULL){
        global_variables_get(command);  // handle echo with variables
    }else if(in != NULL){
        input_redirection(command);
    }else if(out != NULL){
        output_redirection(command);
    }else if(pip != NULL){
        pipes_command(command);
    }else{
        token=strtok(command, " \n");
        char* args[100];
        while(i < 100 && token != NULL){
            args[i]=token;
            token=strtok(NULL, " \n");
            i++;
        }
        args[i]=NULL;
        k=execvp(args[0], args);
        if(k == -1){
            printf("Error execvp()\n");
        }
    }
}

//ektiposi stin othoni
void type_prompt(){
    char command[PATH_MAX];
    char *username;
    username=getlogin();
    if(username!=NULL && getcwd(command,sizeof(command))!=NULL){
        printf("csd5016-hy345sh@%s:%s$",username,command);
    }else{
        printf("getlogin() error OR getpwuid() error.\n");
        exit(0);
    }
}

int main(){
    char cp[100];
    char command[100];
    while (1) {
        type_prompt();
        if(fgets(command, sizeof(command), stdin) == NULL){
            printf("den yparxei eisodos.\n");
            exit(0);
        }
        char *big_cwd=strtok(command, ";\n");
        while (big_cwd != NULL) {
            if(strcmp(big_cwd, "exit") == 0){
                exit(0);
            }
            if(strchr(big_cwd, '=') != NULL){
                strcpy(cp, big_cwd);
                global_variables_set(cp); // set variable in parent process
            } else {
                pid_t pid = fork();
                if (pid == 0) {
                    strcpy(cp, big_cwd);
                    _command(cp); // execute command in child process
                    exit(0);
                } else if (pid < 0) {
                    printf("fork() FAILD.\n");
                    exit(0);
                } else {
                    wait(NULL); // wait for child process
                }
            }
            big_cwd=strtok(NULL, ";\n");
        }
    }
}
