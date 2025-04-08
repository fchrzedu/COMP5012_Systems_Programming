#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
/*

used exel to get path, rather than execlp for file
*/

int main(){
    pid_t pid = fork();
    int stat;

    if (pid < 0) {
        perror(">Forking has failed!\n");
        exit(1);
    }
    else if (pid==0){
        printf(">Child process...\n");
        execl("./child","child",NULL); // NULL for no of args, call all args (using child), from /child
        exit(2);
    }else{
        wait(&stat); // have to wait for execl to complete. Without it parent executed incorrectly
        printf(">Parent process\n");
    }



    return 0;
}