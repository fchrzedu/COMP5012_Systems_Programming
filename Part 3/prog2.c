#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>


int main(){

    pid_t pid = fork(); // holds child PID

    if(pid < 0){
        perror(">Forking has failed\n");
        exit(1);
    }
    else if(pid == 0){
        printf(">This is the child. I am sleeping for 5 seconds zzzz...\n");
        sleep(5);
        printf(">The child has woken up!\n");
    }
    else{
        printf("This is the parent process. Child's sleeping, soon is awake\n");
    }



    return 0;
}