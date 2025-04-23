#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include "sharedlib.h"
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
    return FAIL_RES;

}
void logOpen(){
    openlog(">sharedlib", LOG_PID | LOG_CONS, LOG_USER);
}
/* Sends all block data to daemon*/
ssize_t sendAllData(int fd, const void *data, size_t d_len){
    size_t total_data_sent = 0;
    const uint8_t *dptr = data;
    while(total_data_sent < d_len){
        ssize_t sent = send(fd, dptr+total_data_sent, d_len - total_data_sent, 0);
        if(sent < 1){return -1;}
        total_data_sent += sent;
    }
    return total_data_sent;
}
/* Receives all data from daemon */
ssize_t receiveAllData(int fd, void *buff, size_t length){
    size_t total_received = 0;
    uint8_t *buffptr = buff;
    while(total_received < length){
        ssize_t recvd = recv(fd, buffptr + total_received, length - total_received, 0);
        if(recvd < 1){return -1;}
        total_received += recvd;
    }
    return total_received;
}





uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data){
    logOpen();
    /*
    - Establish IPC connection to daemon.c
    - Send approriate command to daemon.c, including 'ID' and 'secret' (OP code such as SEND_BLOCK)
    - send data_length bytes starting at address 'data' to daemon.c
    - retrieve a response from the daemon.c - indicate whether storing data was succesful
    - return a value to calling program ( a client ) reflecting daemons resposne
    */
    char id_buffer[256] = {0};
    int send_fd = connectDaemon(); /* socket to connect to daemon */
    if(send_fd < 0){
        return handleErr(send_fd);
    }

    /* Send OP code to DAEMON */
    uint8_t opcode = SEND_BLOCK;
    if (sendAllData(send_fd, &opcode, sizeof(opcode)) != sizeof(opcode)){
        syslog(LOG_ERR, "[-sendNewBlock()] failed to send SEND_BLOCK\n");
        return handleErr(send_fd);
    }

    /* Sending ID to daemon.c -  */
    
    strncpy(id_buffer, ID, sizeof(id_buffer) - 1);
    if(sendAllData(send_fd, id_buffer, sizeof(id_buffer)) != sizeof(id_buffer)){
        syslog(LOG_ERR,"[-sendNewBlock()] failed to send ID to daemon\n");
        return handleErr(send_fd);
    }

    /* send 16 byte secret to daemon */
    if(sendAllData(send_fd, secret, 16) != 16){
        syslog(LOG_ERR, "[-sendNewBlock()] failed to send secret to daemon\n");
        return handleErr(send_fd);
    }
    /* sending data length bytes to daemon - prepare buffer */
    if(sendAllData(send_fd, &data_length, sizeof(data_length)) != sizeof(data_length)){
        syslog(LOG_ERR, "[-sendNewBlock()] failed to send data_length to daemon\n");
        return handleErr(send_fd);
    }
    /* send actual data over*/
    if(sendAllData(send_fd, data, data_length) != data_length){
        syslog(LOG_ERR, "[-sendNewBlock()] failed to send data to daemon\n");
        return handleErr(send_fd);
    }

    /* If succesfully sent, await for op code from daemon */
    uint8_t resp;
    if(receiveAllData(send_fd, &resp, sizeof(resp)) != sizeof(resp)){
        syslog(LOG_ERR, "[-sendNewBlock()] failed to receive daemon OPCODE resp\n");
        return handleErr(send_fd);
    }
    close(send_fd);
    return resp;

    
}
uint8_t getBlock(char *ID, uint8_t *secret, uint32_t buffer_size, void *buffer){
    char id_buffer[256] = {0};
    logOpen();
    int get_fd = connectDaemon();
    if(get_fd < 0){
        return handleErr(get_fd);
    }
    /* ----- SENDING OPCODE TO DAEMON*/
    uint8_t opcode = GET_BLOCK;    
    if(sendAllData(get_fd,&opcode,sizeof(opcode)) != sizeof(opcode)){
        syslog(LOG_ERR,"[-]getBlock() failed to send GET_BLOCK\n");
        return handleErr(get_fd);
    }
    /* ----- SENDING ID LENGTH ----- */
    uint32_t id_len = strlen(ID);
    if(sendAllData(get_fd,&id_len,sizeof(id_len)) != sizeof(id_len)){
        syslog(LOG_ERR,"[-]getBlock(): Failed to send data_length\n");
        return handleErr(get_fd);
    }
    /* ----- SENDING ID ----- */
    strncpy(id_buffer,ID, sizeof(id_buffer)-1);
    if(sendAllData(get_fd,id_buffer,sizeof(id_buffer)) != sizeof(id_buffer)){
        syslog(LOG_ERR,"[-]getBlock() failed to send ID\n");
        return handleErr(get_fd);
    }
    /* ----- SENDING SECRET ----- */
    if(sendAllData(get_fd,secret,sizeof(secret)) != sizeof(secret)){
        syslog(LOG_ERR,"[-]getBlock() failed to send secret\n");
        return handleErr(get_fd);
    }

    /* ----- AWAIT OPCODE RESPONSE ----- */
    uint8_t resp = 0;
    if(receiveAllData(get_fd,&resp,sizeof(resp)) != sizeof(resp)){
        syslog(LOG_ERR,"[-]getBlock() failed to receive OPCODE from daemon.c\n");
        return handleErr(get_fd);
    }

    /* ----- ERROR HANDLE ----- */
    if(resp != SUCCESS_RES){
        return handleErr(get_fd);
    }

    uint32_t recvd_data_length = 0;
    if(receiveAllData(get_fd,&recvd_data_length,sizeof(recvd_data_length)) != sizeof(recvd_data_length)){
        syslog(LOG_ERR,"[-]getBlock() failed to receive data_length\n");
        return handleErr(get_fd);
    }
    /* ----- BUFFER SIZE ----- */
    if(recvd_data_length > buffer_size){
        syslog(LOG_ERR,"[-]getBlock() failed to appropriate buffer size \n");
        return handleErr(get_fd);
    }

    /* ----- RECEIVE DATA -----*/
    if(receiveAllData(get_fd,buffer,recvd_data_length) != (ssize_t)recvd_data_length){
        syslog(LOG_ERR,"[-]getBlock() failed to receive data\n");
        return handleErr(get_fd);
    }
    close(get_fd);
    return SUCCESS_RES;

    
}


