#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> // wait()
#include <errno.h> // perror



int main(){
    pid_t pid = fork(); //fork

    if(pid<0){
        perror(">Forking has failed!\n");        
    }
    else if(pid == 0){
        execl("./child","child",NULL); // replace with ./child process
        exit(1);
    }else{
        wait(NULL); // NULL waits for child
        printf(">Parent proces. Child has finished!\n");
    }






    return 0;
}