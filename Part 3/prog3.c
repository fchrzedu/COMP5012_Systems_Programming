#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> // wait()
#include <errno.h> // perror

int main(){
    pid_t pid = fork();
    int stat; // capture

    if(pid<0){perror("->Forking has failed!\n");return 1;}
    else if(pid == 0){//child
        printf("->This is the child process. Hello!\n");
        exit(2); // arbitary integer for an exit status              
    }
    else{//parent
        wait(&stat);
        if(WIFEXITED(stat)){
            printf("->This is the parent process. Child has exited with status: %d\n",WEXITSTATUS(stat)); // check whether child exited, and print its status
        }    
    }
    printf("Bye\n");

    






    return 0;
}