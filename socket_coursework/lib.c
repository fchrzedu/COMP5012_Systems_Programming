
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include "lib.h"

ssize_t receiveData(int sockfd, void *buffer, size_t len){
    size_t total_received = 0;
    const uint8_t *bufferptr = buffer;
    while(total_received < len){
        ssize_t received = recv(sockfd, bufferptr + total_received, len - total_received, 0);
        if(received<1){return -1;}
        total_received += received;
    }
    return total_received;
}
ssize_t sendData(int sockfd, const void *data, size_t data_length){
    size_t total_sent = 0;
    const uint8_t *dataptr = data;
    while(total_sent < data_length){
        ssize_t sent = send(sockfd, dataptr + total_sent, data_length - total_sent, 0);
        if(sent < 1){return -1;}
        total_sent += sent;
    }
    return total_sent;
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
    openlog(">lib", LOG_PID | LOG_CONS, LOG_USER);
}

uint8_t handleError(int fd){
    close(fd);
    return FAIL;
}

uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data){
    /*
    1. OPCODE - done
    2. ID - done
    3. SECRET - done 
    4. DATA LENGTH
    5. DATA
    */
    if(!ID || !secret || !data || data_length == 0){
        syslog(LOG_ERR,"[-]sendNewBlock: invalid args\n");
        return FAIL;
    }
    /* -- connect to daemon -- */
    logOpen();    
    int sendfd = connectDaemon();
    if(sendfd < 0){
        return handleError(sendfd);
    }   

    /* -- send op code -- */
    uint8_t opcode = SEND;
    sendData(sendfd,&opcode,sizeof(opcode));

    /* ----- prepare and send ID ----- */
    char idbuffer[256] = {0};
    strncpy(idbuffer, ID, sizeof(ID) - 1); /* '\0 */
    sendData(sendfd,idbuffer,sizeof(idbuffer));
    
    /* -- send secret -- */
    sendData(sendfd,secret,16);

    /* -- send data length --*/
    sendData(sendfd, &data_length, sizeof((data_length)));

    /* -- send data -- */
    sendData(sendfd, data, data_length);

    /* -- await daemon response code --*/
    uint8_t resp = receiveData(sendfd,&resp,sizeof(resp));

    close(sendfd);
    return resp;
}
uint8_t getBlock(char *ID, uint8_t *secret, uint32_t buffer_size, void *buffer);
