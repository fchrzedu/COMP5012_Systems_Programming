#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> // wait()
#include <errno.h> // perror

int main(){
    printf(">Child process. PID is %d\n",getpid());
    return 0;
}