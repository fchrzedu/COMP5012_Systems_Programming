#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

/*
1. Checks if forking has failed and throws error along exit code
2. Prints out child PID and its parent PID
3. Prints out parent PID, the parents parent PID, and the child PID (fork always stores child PID)
*/



int main (){
    pid_t pid = fork();
    if(pid < 0){ // error
        perror(">Forking has failed!\n");
        exit(1);
    }
    else if (pid == 0){//child    
       printf("Child process. My PID == %d, parent PID == %d\n",getpid(),getppid());
    }
    else{//parent    
        printf("Parent process. My PID == %d, my parent's PID == %d, child PID == %d\n",getpid(),getppid(),pid);
    }

    return 0;
}