#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
/*
1. Child terminates with status 21 (randomly chosen num)
2. Parent waits on child and captures the childs exist status, prints it then terminates
*/
int main(){

    int status;
    pid_t pid = fork();
    if(pid<0){perror(">Forking has failed!\n");exit(1);}

    else if(pid == 0){
        printf(">Hi, i'm child! I'll be terminating with status 21.\n");
        exit(21);
    }
    else{
        wait(&status);// wait on status address
        if(WIFEXITED(status)){ // if succesfully captured an exit code from child process
            printf(">Hi i'm parent. Child has exited with code %d\n",WEXITSTATUS(status));
            
        }
        printf(">Parent is now terminating.\n");
    }
}