#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
int main(int argc, char* argv[])
{

    // make two process which run same
    // program after this instruction
    pid_t p = fork();
    if(p<0){
      perror("fork fail");
      return 1;
    }else if(p==0){ // succesful forking for child if zero
        printf("Child process PID = %d, Parent process PID = %d\n",getpid(),getppid());
    }
    else{ // not zero and not -ve means parent
        printf("Parent process PID = %d, Child process PID = %d\n",getppid(),getpid());
    }
   
    return 0;
}