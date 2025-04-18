#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include "sharedlib.h"

int handleErr(int sockfd){
    close(sockfd);
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
    /* send()
    > sd = socked descr
    > const void *msg = message
    > size_t = sizeof(*msg)
    > flags = 0 (always zero)
    > return = number of bytes actually send - error checking here to make sure complete delivery
    */
    logOpen();
    int bsockfd = connectDaemon();
    if(bsockfd<0){return RES_FAILURE;}

    /* used to let the daemon know of sending datablock - only 1 byte*/
    uint8_t cmd = SEND_BLOCK;   

    /* Sending all data here w/ error codes & checks */
    if(send(bsockfd, &cmd, sizeof(cmd), 0) != sizeof(cmd)){
        syslog(LOG_ERR, "[-]Failed to send cmd_block_send to daemon\n");
        return handleErr(bsockfd);
    }
    uint32_t ID_len = strlen(ID) + 1; /* accounts for \0 */
    if(send(bsockfd, ID, ID_len, 0) != ID_len){
        syslog(LOG_ERR, "[-]ID send error\n");
        return handleErr(bsockfd);
    }
    /* len = 16 -> MUST be 16 bytes, no other value */
    if(send(bsockfd, secret, 16, 0) != 16){
        syslog(LOG_ERR, "[-]Secret send error\n");
        return handleErr(bsockfd);
    }
    if(send(bsockfd, &data_length, sizeof(data_length), 0) != sizeof(data_length))
    {
        syslog(LOG_ERR, "[-]data_len send error\n");
        return handleErr(bsockfd);
    }
    if(send(bsockfd, data, data_length, 0) != data_length){
        syslog(LOG_ERR, "[-]Data sending error\n");
        return handleErr(bsockfd);
    }

    /* Awaiting daemon response*/
    /* recv()
    > sd = sockfd
    > *msg = data recieved
    > msg_len = sizeof(msg)
    > flags = 0*/

    uint8_t res; /* res = response */
    if(recv(bsockfd, &res, sizeof(res), 0) != sizeof(res)){
        syslog(LOG_ERR, "[-]Recv() from daemon error\n");
        return handleErr(bsockfd);
    }
    close(bsockfd);
    syslog(LOG_INFO, "[+] sendNewBlock() reponse = %d\n", res);
    return res;
}


uint8_t getBlock(char *ID, uint8_t *secret, uint32_t buffer_size, void *buffer) 


