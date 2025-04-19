#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include "sharedlib.h"

uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data){
    /*
    - Establish IPC connection to daemon.c
    - Send approriate command to daemon.c, including 'ID' and 'secret'
    - send data_length bytes starting at address 'data' to daemon.c
    - retrieve a response from the daemon.c - indicate whether storing data was succesful
    - return a value to calling program ( a client ) reflecting daemons resposne
    */
}

uint8_t handleErr(int fd){
    close(fd);
    return RES_FAILURE;

}
int connectDaemon(){
    logOpen();
    struct sockaddr_un address;
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd < 0){
        syslog(LOG_ERR, "[-]Socket creation failed\n");
        return -1;
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) -1);
    if(connect(sockfd, (struct sockaddr* )&address, sizeof(address))<0){
        syslog(LOG_ERR, "[-]Connceting to daemon failed\n");
        close(sockfd);
        return -1;
    }
    return sockfd;
}
void logOpen(){
    openlog(">sharedlib", LOG_PID | LOG_CONS, LOG_USER);
}

