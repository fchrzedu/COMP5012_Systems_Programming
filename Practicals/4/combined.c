#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


int main(){
    int fd;
    char *myfifo = "/tmp/myfifo";

    mkfifo(myfifo,0666); // creates fifo file

    pid_t pid = fork(); // create child process

    if (pid < 0){
        perror(">Error when forking\n");
        return 1;
    }
    else if (pid == 0){//child process 
        printf(">Currently in child process.\n");
        char str1[80]; // assuming this is a buffer of size 80
        fd = open(myfifo,O_RDONLY); // open myfifo as read only
        read(fd,str1,80);; // assuming this reads using fd, stores in str1, max size 80
        close(fd); // close fd (closing instance of open fifo)
    }else {//parent process after child
    
        char str2[80]; // array to store buffer
        fd = open(myfifo,O_WRONLY); // parent process opens fifo to write to it
        fgets(str2,80,stdin);
        write(fd,str2,strlen(str2) + 1);
        close(fd);
    }





}