#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include "sharedlib.h"

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


/* helper function to send data to daemon down socket */
ssize_t sendAllData(int fd, const void *d, size_t d_len){
    ssize_t sent_total = 0;
    const char *dataptr = (const char*)d;
    while(sent_total < d_len){
        ssize_t sent = send(fd, dataptr + sent_total, d_len - sent_total, 0);
        if(sent<1){
            syslog(LOG_ERR, "[-sendAllData()] Error sending data to daemon.c");
            return -1;
        }
        sent_total +=sent;
    }return 0;
}

ssize_t receiveAllData(int fd, void *buff, size_t length){
    ssize_t recv_counter = 0;
    
    while(recv_counter < length){
        ssize_t received = recv(fd, (char*)buff + recv_counter, length - recv_counter, 0);
        if(received < 1){
            syslog(LOG_ERR, "[-receiveAllData()]Error receiving data\n");
            return -1;
        }
        recv_counter += received;
    }
    return recv_counter;
}

/* Uses op & return codes to return status of block - artibary values*/
uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data) {
    logOpen();

    if(ID == NULL || secret == NULL || data == NULL || data_length == 0){
        syslog(LOG_ERR,"[-]Empty sendNewBlock() args\n");
        return RES_FAILURE;
    }

    uint8_t s_cmd = CMD_SEND_BLOCK;
    int send_block_fd = connectDaemon();
    if (send_block_fd < 0) {
        return RES_FAILURE;
    }

    char fixed_id[256] = {0};
    strncpy(fixed_id, ID, sizeof(fixed_id) - 1);
    if (sendAllData(send_block_fd, fixed_id, sizeof(fixed_id)) != 0) {
        return handleErr(send_block_fd);
    }


    if (sendAllData(send_block_fd, secret, 16) != 16) {
        return handleErr(send_block_fd);
    }

    if (sendAllData(send_block_fd, &data_length, sizeof(data_length)) != sizeof(data_length)) {
        return handleErr(send_block_fd);
    }

    if (sendAllData(send_block_fd, data, data_length) != data_length) {
        return handleErr(send_block_fd);
    }

    uint8_t resp;
    if (receiveAllData(send_block_fd, &resp, sizeof(resp)) != sizeof(resp)) {
        return handleErr(send_block_fd);
    }

    close(send_block_fd);
    syslog(LOG_INFO, "[+]sendNewBlock() successful: %d", resp);
    return resp;

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
    
    uint8_t g_cmd = CMD_GET_BLOCK;
    if (sendAllData(get_blockfd, &g_cmd, sizeof(g_cmd)) < 0) {
        syslog(LOG_ERR, "getBlock(): Failed to send command");
        return handleErr(get_blockfd);
    }

    char fixed_id[256] = {0};
    strncpy(fixed_id, ID, sizeof(fixed_id) - 1);
    if (sendAllData(get_blockfd, fixed_id, sizeof(fixed_id)) != 0) {
        syslog(LOG_ERR, "getBlock(): Failed to send ID");
        return handleErr(get_blockfd);
    }
    syslog(LOG_INFO, "getBlock(): Sending secret, fd = %d", get_blockfd);

    if (secret == NULL) {
    syslog(LOG_ERR, "getBlock(): Secret pointer is NULL");
        return handleErr(get_blockfd);
    }

    if (sendAllData(get_blockfd, secret, 16) != 16) {
        syslog(LOG_ERR, "getBlock(): Failed to send secret");
        return handleErr(get_blockfd);
    }

    syslog(LOG_INFO, "getBlock(): Sending buffer_size = %u", buffer_size);
    if (sendAllData(get_blockfd, &buffer_size, sizeof(buffer_size)) != sizeof(buffer_size)) {
        syslog(LOG_ERR, "getBlock(): Failed to send buffer_size");
        return handleErr(get_blockfd);
    }
    else {
        syslog(LOG_INFO, "getBlock(): buffer_size sent successfully");
    }

    uint8_t resp;
    if (receiveAllData(get_blockfd, &resp, sizeof(resp)) != sizeof(resp)) {
        return handleErr(get_blockfd);
    }

    if (resp != RES_SUCCESS) {
        syslog(LOG_ERR, "[-]Daemon failed response code = %d\n", resp);
        close(get_blockfd);
        return resp;
    }

    if (receiveAllData(get_blockfd, buffer, buffer_size) != buffer_size) {
        return handleErr(get_blockfd);
    }

    close(get_blockfd);
    syslog(LOG_INFO, "[+]getBlock() success. Received %d bytes\n", buffer_size);
    return RES_SUCCESS;

}


