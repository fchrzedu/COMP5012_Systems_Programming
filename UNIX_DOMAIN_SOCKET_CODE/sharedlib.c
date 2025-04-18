#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include "sharedlib.h"

int handleErr(int fd){
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

/* Uses op & return codes to return status of block - artibary values*/
uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data) {
    logOpen();

    if(ID == NULL || secret == NULL || data == NULL || data_length == 0){
        syslog(LOG_ERR,"[-]Empty sendNewBlock() args\n");
        return RES_FAILURE;
    }

    uint8_t s_cmd= CMD_SEND_BLOCK;
    int send_block_fd = connectDaemon();
    uint32_t IDlen = strlen(ID);

    /* sending op code to daemon */
    if(send(send_block_fd, s_cmd, sizeof(s_cmd), 0) != sizeof(s_cmd)){
        syslog(LOG_ERR, "[-]CMD_SEND_BLOCK not sent\n");
        return handleErr(send_block_fd);
    }

    /* sending ID - len +1 to account for \0 at end of ID */
    if(send(send_block_fd, ID, IDlen + 1, 0) != IDlen ){
        syslog(LOG_ERR, "[-sendnewBlock()]Error sending ID to daemon.c\n");
        return handleErr(send_block_fd);
    }

    /* sending secret - 16 due to the only accepted num of bytes */
    if(send(send_block_fd, secret, 16, 0) != 16){
        syslog(LOG_ERR, "[-sendnewBlock()]Error sending secret (16) to daemon.c\n");
        return handleErr(send_block_fd);
    }

    /* sending data length over to daemon */
    if(send(send_block_fd, &data_length, sizeof(data_length), 0) != sizeof(data_length)){
        syslog(LOG_ERR, "[-sendnewBlock()]Error sending data_length to daemon.c\n");
        return handleErr(send_block_fd);
    }
    /*
    - signed counter = don't want 'negative' bytes sent
    - dataptr points to data
    - sends data byte by byte, ensures delivery (send() doesn't always send all )*/
    ssize_t sent_counter = 0;
    const char *dataptr = (const char *)data;
    while((uint32_t)sent_counter < data_length){
        ssize_t sent = send(send_block_fd, dataptr + sent_counter, data_length - sent_counter, 0);
        if(sent < 1){
            syslog(LOG_ERR, "[-sendnewBlock()]Error sending data [dataptr] to daemon.c\n");
            return handleErr(send_block_fd;)
        }
        sent_counter += sent;
    }
    uint8_t resp;
    if(recv(send_block_fd, &resp, sizeof(resp), 0)!=sizeof(resp)){
        syslog(LOG_ERR, "[-sendnewBlock()]Error response [recv()] from daemon.c");
        return handleErr(send_block_fd);
    }

    close(send_block_fd);
    syslog(LOG_INFO,"[+sendNewBlock()] succesful : %d",resp);return resp;

}


uint8_t getBlock(char *ID, uint8_t *secret, uint32_t buffer_size, void *buffer){
    logOpen();
    if (ID == NULL || secret == NULL || buffer == NULL || buffer_size == 0) {
        syslog(LOG_ERR, "[-getBlock()] Invalid arguments passed to getBlock()\n");
        return RES_FAILURE;
    }   

    /* verify connectivity daemon.c*/
    int get_blockfd = connectDaemon();    
    if (get_blockfd < 0) {
        syslog(LOG_ERR, "[-getBlock()]Error connecting to daemon [getBlock()]\n");
        return RES_FAILURE;
    }
    
    /* send command over */
    int g_cmd = CMD_GET_BLOCK;
    if(send(get_blockfd, &g_cmd, sizeof(g_cmd), 0) != sizeof(g_cmd)){
        syslog(LOG_ERR, "[-getBlock()] CMD_GET_BLOCK send() error\n");
        return handleErr(get_blockfd);
    }

    /* send ID with '\0' */
    uint32_t IDlen = strlen(ID) + 1;
    if(send(get_blockfd, ID, IDlen, 0)!= IDlen){
        syslog(LOG_ERR,"[-getBlock()] Error sending ID to daemon.c\n");
        return handleErr(get_blockfd);
    }   
    /* send secret (16 bytes)*/
    if(send(get_blockfd, secret, 16, 0) != 16){
        syslog(LOG_ERR,"[-getBlock()] Error sending secret (16) to daemon.c\n");
        return handleErr(get_blockfd);
    }

    /* send buff size */
    if(send(get_blockfd, &buffer_size, sizeof(buffer_size), 0) != sizeof(buffer_size)){
        syslog(LOG_ERR,"[-getBlock()] Error sending buffer size to daemon.c\n");
        return handleErr(get_blockfd);
    }

    /* retrieve cmd from daemon */
    uint8_t resp;
    if(recv(get_blockfd, &resp, sizeof(resp), 0) != sizeof(resp)){
        syslog(LOG_ERR,"[-getBlock()] Error recv() code from daemon.c\n");
        return handleErr(get_blockfd);
    }

    if(resp != RES_SUCCESS){
        syslog(LOG_ERR,"[-getBlock()] daemon.c failed response code = %d\n",resp);
        close(get_blockfd);
        return resp; /* could be a different response code each time*/
    }

    /* recv() data when success */
    ssize_t recv_counter = 0; 
    while((uint32_t)recv_counter < buffer_size){
        ssize_t recv = recv(get_blockfd, (char*)buffer + recv_counter, buffer_size - recv_counter, 0);

        /* error check for either incomplete receiving or halt mid process */
        if(recv < 0){
            syslog(LOG_ERR, "[-getBlock()] Error receiving data from daemon.c\n");
            return handleErr(get_blockfd);
        }
        if(recv == 0){
            syslog(LOG_ERR, "[-getBlock()] Connection terminated!\n");
            return handleErr(get_blockfd);
        }
        recv_counter += recv;
    }

    close(get_blockfd);
    syslog(LOG_INFO, "[+] getBlock() success. Recieved %d bytes\n",recv_counter);
    return RES_SUCCESS;



}


