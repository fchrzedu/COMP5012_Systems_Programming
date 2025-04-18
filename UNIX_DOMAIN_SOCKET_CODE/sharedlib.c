#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include "sharedlib.h"
int connectDaemon(){
    struct sockaddr_un address;
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if(sockfd < 0){
        syslog(LOG_ERR, "[-]Socket creation failed\n");
        return -1;
    }

    memset(&address, 0, sizeof(address));
    address.sun_family = AD_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) -1);
    if(connect(sockfd, (struct sockaddr* )&address, sizeof(address))<0){
        syslog(LOG_ERR, "[-]Connceting to daemon failed\n");
        close(sockfd);
        return -1;
    }
    return sockfd;
}
void openLog(){
    openlog(">sharedlib", LOG_PID | LOG_CONS, LOG_USER);
}
uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data) {
    /* send()
    > sd = socked descr
    > const void *msg = message
    > size_t = sizeof(*msg)
    > flags = 0 (always zero)
    > return = number of bytes actually send - error checking here to make sure complete delivery
    */
    openLog();
    int bsockfd = connectDaemon();
    if(bsockfd<0){return RES_FAILURE;}

    /* used to let the daemon know of sending datablock - only 1 byte*/
    uint8_t cmd = CMD_SEND_BLOCK;   


    if(send(bsockfd, &cmd, sizeof(cmd), 0) != sizeof(cmd)){
        syslog(LOG_ERR, "[-]Failed to send cmd_block_send to daemon\n");
        close(sockfd);
        return RES_FAILURE;
    }
    uint32_t ID_len = strlen(ID) + 1; /* accounts for \0 */
    if(send(sockfd, ID, ID_len, 0) != ID_len){
        syslog(LOG_ERR, "[-]ID send error\n");
    }

}
