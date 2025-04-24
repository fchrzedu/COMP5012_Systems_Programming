#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include "lib.h"

#define LOG_FILE_PATH "/tmp/lib.txt"
/* -- client -> lib -> socket -> daemon -- */
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

uint8_t handleErr(int fd){
    close(fd);
    return FAIL;

}
void logOpen(){
    openlog("LIB", LOG_PID | LOG_CONS, LOG_USER);
}


uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data){
    /*
    1. send op code YES
    2. send ID YES
    3. send secret
    4. send data length
    5. send data
    */
    logOpen();
    int sockfd = connectDaemon();
    if(sockfd < 0){return FAIL;}
    
    
    /* -- send op code -- */
    uint8_t opcode = SEND;
    if(send(sockfd,&opcode,sizeof(opcode),0) != sizeof(opcode)){
        syslog(LOG_ERR,"[-]sendNewBlock() failed to send opcode\n");
        return handleErr(sockfd);
    }
    /* - send ID and its length - */
    uint8_t id_length = strlen(ID);
    if(send(sockfd, &id_length, sizeof(id_length), 0) != sizeof(id_length)){
        syslog(LOG_ERR,"[-]sendNewBlock() failed to send ID length\n");
        return handleErr(sockfd);
    }
    char idsend[256] = {0};
    strncpy(idsend, ID, sizeof(idsend) - 1);
    if(send(sockfd, idsend ,sizeof(idsend),0) != sizeof(idsend)){
        syslog(LOG_ERR,"[-]sendNewBlock() failed to send ID\n");
        return handleErr(sockfd);
    }
    /* - send secret - */
    if(send(sockfd, secret, 16, 0) != 16){
        syslog(LOG_ERR,"[-]sendNewBlock() failed to send secret\n");
        return handleErr(sockfd);
    }
    /* - send data_length - */
    if(send(sockfd,&data_length,sizeof(data_length),0) != sizeof(data_length)){
        syslog(LOG_ERR,"[-]sendNewBlock() failed to send data_length\n");
        return handleErr(sockfd);
    }
    /* - send data -*/
    if(send(sockfd,data,data_length,0) != data_length){
        syslog(LOG_ERR,"[-]sendNewBlock() failed to send data\n");
        return handleErr(sockfd);
    }
    

    uint8_t response = 0;
    ssize_t received = recv(sockfd, &response, sizeof(response),0);
    if(received != sizeof(response)){
        syslog(LOG_ERR,"[-]sendNewBlock() failed to receive daemon response\n");
        return handleErr(sockfd);
    }
    close(sockfd);
    return response;
    

}
uint8_t getBlock(char *ID, uint8_t *secret, uint32_t buffer_size, void *buffer){
    /*
    1. send op code
    2. send Id length
    3. send ID
    4. send secret 
    5. await response
    */
    logOpen();
    int sockfd = connectDaemon();
    if(sockfd < 0){return FAIL;}

    
    uint8_t opcode = GET;
    if(send(sockfd,&opcode,sizeof(opcode),0) != sizeof(opcode)){
        syslog(LOG_ERR,"[-]sendNewBlock() failed to send opcode GET\n");
        return handleErr(sockfd);
    }

    /* -- send ID length -- */
    uint8_t id_length = strlen(ID);
    if(send(sockfd, &id_length, sizeof(id_length), 0) != sizeof(id_length)){
        syslog(LOG_ERR,"[-]getBlock() failed to send ID length\n");
        return handleErr(sockfd);
    }
    /* -- send ID -- */
    char idsend[256] = {0};
    strncpy(idsend, ID, sizeof(idsend) - 1);
    if(send(sockfd, idsend ,sizeof(idsend),0) != sizeof(idsend)){
        syslog(LOG_ERR,"[-]getBlock() failed to send ID\n");
        return handleErr(sockfd);
    }
    /* -- send secret -- */
    if(send(sockfd, secret, 16, 0) != 16){
        syslog(LOG_ERR,"[-]getBlock() failed to send secret\n");
        return handleErr(sockfd);
    }

    /* -- await response -- */
    uint8_t response = 0;
    if(recv(sockfd,&response,sizeof(response),0) != sizeof(response)){
        syslog(LOG_ERR,"[-]getBlock() failed to receive response\n");
        return handleErr(sockfd);
    }
    /* -- handle success response -- */
    if(response != SUCCESS){
        syslog(LOG_ERR,"[-]getBlock() response: %d",response);
        close(sockfd);
        return response;
    }
    uint32_t data_length = 0;
    if(recv(sockfd,&data_length,sizeof(data_length),0) != sizeof(data_length)){
        syslog(LOG_ERR,"[-]getBlock() failed to receive data_lengt\n");
        return handleErr(sockfd);

    }
    /*
    uint32_t totalrecvd = 0;
    uint8_t *dptr = (uint8_t*)buffer;
    while(totalrecvd < data_length){
        ssize_t recvd = recv(sockfd, dptr + totalrecvd, data_length - totalrecvd, 0);
        if(recvd < 1){
            syslog(LOG_ERR,"[-]getBlock() failed to receive all data\n");
            return handleErr(sockfd);
        }
        totalrecvd += recvd;
    }*/
    
    uint32_t total_read = 0;
    while(total_read < data_length){
        ssize_t recvd = recv(sockfd, buffer + total_read, data_length - total_read, 0);
        if(recvd < 1){
            syslog(LOG_ERR,"[-]getBlock() failed to receive all of data\n");
            return handleErr(sockfd);
        }
        total_read += recvd;
        
    }
    close(sockfd);
    return SUCCESS;

}


