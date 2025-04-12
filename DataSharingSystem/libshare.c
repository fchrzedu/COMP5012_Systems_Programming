#include "libshare.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h> /* log() */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <syslog.h>


#define PORT 9002 
#define LOOPBACK_IP "127.0.0.1"

void socketFail(){
    syslog(LOG_ERR,"[-]Client socket creation failed!\n");
        exit(EXIT_FAILURE);
    }
void callLog(){
    openlog("API shared library", LOG_PID, LOG_DAEMON);
}

uint8_t sendNewBlock(const char *ID, const uint8_t *secret, const uint32_t data_length, const char *data){
    callLog();
    /* 
    - ID = string of callers choice
    - secret = 16byte array unsigned bytes of callers choice
    - data = ptr to data structure in memory (buffer)
    - data_length = len(data)
    - buffer_size = allocated buffer memory (unoccupied memory)


    - Establish IPC connection to daemon
    - send appropriate comand to it, including ID and secret
    - send data_length bytes starting at address data to it
    - retrieve daemon response, indicate whether storing data was succesful
    - return value reflecting daemon response
    */
    int blockfd = connect_to_daemon();

    return 0;

}
uint8_t getBlock(const char *ID, const uint8_t *secret, const uint32_t buffer_size, const char *buffer){

    return 0;
}



int connect_to_daemon(){
    callLog();
    int c_sock_fd = socket(AF_INET, SOCK_STREAM, 0); /* Create socket */
    if(c_sock_fd < 0){socketFail();}     
    

    struct sockaddr_in my;
    my.sin_family = AF_INET;
    my.sin_port = htons(PORT);
    my.sin_addr.s_addr = inet_addr(LOOPBACK_IP);

    /* Connect to the server */
    if (connect(c_sock_fd, (struct sockaddr*)&my, sizeof(my)) < 0) {
        syslog(LOG_ERR,"[-]Connection to the server failed!\n");        
        close(c_sock_fd);
        exit(1);
    }
    printf("[+] Succesfully connected to the TCP Daemon on PORT: %d, address: %d\n",PORT,LOOPBACK_IP);

    close(c_sock_fd);

    return 0;
}
