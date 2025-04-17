#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "sharedlib.h"


uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data){

    int sockfd = connectDaemon();
    if(sockfd < 0){return -1;}

    
}
int connectDaemon(){
    
    struct sockaddr_un address;
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("connect");
        close(sockfd);
        return -1;
    }
    return sockfd;
}