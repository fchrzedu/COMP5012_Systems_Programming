#include "libshare.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h> /* log() */

#define PORT 9002
#define LOOPBACK "127.0.0.1" /* Retrieved from telnet localhost 9002*/
#define BUFFER 1024


/* Creates a socket attaching to the daemon */
void log(const char *message){
    syslog(LOG_ERR, "%s", message);
}
int daemonConnect(){
    
    int csock = socket(AF_INET, SOCK_STREAM, 0); /* Create socket */
    struct sockaddr_in server_address; /* Server address structure */
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT); // port num
    server_address.sin_addr.s_addr = inet_addr(LOOPBACK); // accept any connections

    if (connect(csock, (struct sockaddr*)&daemon_addr, sizeof(daemon_addr)) < 0) {
        perror("Connection to daemon failed");
        close(csock);
        return -1;
    }
    printf("Connected!\n");


}
uint8_t sendNewBlock(const char *ID, const uint8_t *secret, const uint32_t data_length, const char *data){
    blockSock = daemonConnect(); /* Establish socket communication with daemon */
    if(blockSock<0){
        log("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("-----Connected to daemon\n-----");

    /* 
    * Allocates space to maximum buffer
    * Preprares the string into a char array
    * Useage due to requiring \0 at the end
    */
    char buffer[BUFFER];
    snprintf(buffer, sizeof(buffer),"SENDING:%s%s%u%s", ID, secret, data_length, data);



    
}
