#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>



int main(){
    pid_t pid = fork();
    if(pid < 0){ // error
        perror(">Forking has failed!\n");
        exit(1);
    }
    if(pid==0){
        printf(">You have succesfully used execl!\n>This is child. My PID is == %d\n",getpid());
        
    }
}