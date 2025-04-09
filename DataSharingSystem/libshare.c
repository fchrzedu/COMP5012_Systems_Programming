#include "libshare.h"
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h> /* log() */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 9002 
#define LOOPBACK_IP "127.0.0.1"


uint8_t sendNewBlock(const char *ID, const uint8_t *secret, const uint32_t data_length, const char *data){}
uint8_t getBlock(const char *ID, const uint8_t *secret, const uint32_t buffer_size, const char *buffer){}


int connect_to_daemon(){
    
    int c_sock_fd = socket(AF_INET, SOCK_STREAM, 0); /* Create socket */
    if(c_sock_fd < 0){
        perror("[-] Client socket creation failed!\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in my;
    my.sin_family = AF_INET;
    my.sin_port = htons(PORT);
    my.sin_addr.s_addr = inet_addr(LOOPBACK_IP);

    /* Connect to the server */
    if (connect(c_sock_fd, (struct sockaddr*)&my, sizeof(my)) < 0) {
        perror("[-] Connection to the server failed!\n");
        close(c_sock_fd);
        exit(1);
    }
    printf("[+] Connected to the server successfully.\n");

    close(c_sock_fd);

    return 0;
}
