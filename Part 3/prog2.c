#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(){

    pid_t p = fork(); // new process
    if(p < 0){
        printf("-----Unable to fork!-----\n");
        return 1;
    }

    else if(p == 0){//succesful fork, currently in child
        printf("-> Currently in child process. Child PID = %d, my parent PID is %d\n", getpid(), getppid());
        sleep(4); // child sleeps
        printf("-> Child now awake from sleep. My PID is %d, my parent left and their PID is %d\n", getpid(), getppid());
        printf("-> INIT REAPED CHILD PROCESS\n");
    }
    else{//parent process - exits before child

        printf("-> I am the parent process, my PID is %d, my child's PID is %d\n",getppid(),getpid());
        printf("-> Exiting, making child process an orphan\n");  // init reaps orphan child
        return 0;
    }





    return 0;
}