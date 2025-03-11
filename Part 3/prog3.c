#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> // wait()
#include <errno.h> // perror

int main(){
    pid_t pid = fork();
    int stat; // capture

    if(pid<0){perror(">Forking has failed!\n");return 1;}
    else if(pid == 0){//child
        printf(">This is the child process. Hello and goodnight!\n");sleep(5);         
        printf(">This is the child process. I have woken up, exiting now...\n");
        exit(200); // arbitary integer for an exit status              
    }
    else{ //parent
        wait(&stat);
        if(WIFEXITED(stat)){ // check whether child has exited. If so, capture its exit status
            printf(">This is the parent process. Child has exited with status: %d\n",WEXITSTATUS(stat)); 
        }    
    }
    printf(">Bye!\n");

    






    return 0;
}