#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <stdbool.h>
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

bool sendOpCode(int fd, uint8_t opcode){
    if(send(fd, &opcode, sizeof(opcode),0 ) != sizeof(opcode)){
        syslog(LOG_ERR,"[-]sendOpCode() failed to send opcode\n");
        return false;
    }
    return true;
}
bool sendIDAndLength(int fd, char*ID, int flag){
    /*
    flag = 1 : send ID length
    flag = 2: send ID*/
    if(flag == 1){
        uint8_t id_l = strlen(ID);
        if(send(fd, &id_l, sizeof(id_l), 0) != sizeof(id_l)){
            syslog(LOG_ERR,"[-]sendIDAndLength() failed to send ID length\n");
            return false;
        }
    }
    else if (flag == 2){
        uint8_t id_l = strlen(ID);
        
        if(send(fd, ID, id_l,0) != id_l){
            syslog(LOG_ERR,"[-]sendIDAndLength() failed to send ID\n");
            return false;
        }

    }
    else{
        syslog(LOG_ERR,"[-]lib.c::sendIDAndLength unrecognised flag: %u", flag);
        return false;
    }
    
    return true;
}
bool sendSecret(int fd, uint8_t *secret){
    if(send(fd, secret, 16, 0 ) != 16){
        syslog(LOG_ERR,"[-]sendSecret() failed to send secret\n");
        return false;
    }
    return true;
}

bool sendDataLength(int fd, uint32_t d_len){
    if(send(fd, &d_len, sizeof(d_len), 0) != sizeof(d_len)){
        syslog(LOG_ERR,"[-]sendDataLength() failed to send data_length\n");
        return false;
    }
    return true;
}
bool sendActualData(int fd, void *d, uint32_t d_len){
    if(send(fd, d, d_len,0) != d_len){
        syslog(LOG_ERR,"[-]sendActualData() failed to send data\n");
        return false;
    }
    return true;
}
uint8_t receiveResponse(int fd){
    uint8_t resp = 0;
    if(recv(fd, &resp, sizeof(resp),0) != sizeof(resp)){
        syslog(LOG_ERR,"[-]receiveResponse() failed to receive daemon response\n");
        return handleErr(fd);
    }
    return resp;
}
bool receiveDataLength(int fd, uint32_t *d_len){
    if(recv(fd, d_len, sizeof(*d_len),0) != sizeof(*d_len)){
        syslog(LOG_ERR,"[-]receiveDataLength() failed to receive data_length\n");
        return false;
    }
    return true;
}
bool receiveActualData(int fd, void *b, uint32_t check_len){
    uint32_t total_read = 0;
    while(total_read < check_len){
        ssize_t recvD = recv(fd, b+total_read, check_len - total_read, 0);
        if(recvD < 1){
            syslog(LOG_ERR,"[-]receiveActualData failed to receive data\n");
            return false;
        }
        total_read += recvD;
    }
    return true;
}

uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data){
    /*
    1. send op code YES
    2. send ID YES
    3. send secret
    4. send data length
    5. send data
    */
    logOpen();int sockfd = connectDaemon();if(sockfd < 0){return FAIL;}   
    /* -- send flag 'SEND' -- */
    if(!sendOpCode(sockfd, (uint8_t)SEND)){ return handleErr(sockfd);}
    /* - send ID length followed by ID - */
    if(!sendIDAndLength(sockfd, ID, 1)){return handleErr(sockfd);}
    if(!sendIDAndLength(sockfd, ID, 2)){return handleErr(sockfd);}

    /* - send secret - */
    if(!sendSecret(sockfd, secret)){return handleErr(sockfd);}
    
    /* - send data_length - */
    if(!sendDataLength(sockfd, data_length)){return handleErr(sockfd);}
    if(!sendActualData(sockfd, data, data_length)){return handleErr(sockfd);}  

    uint8_t response = receiveResponse(sockfd);    
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
    logOpen();int sockfd = connectDaemon();  if(sockfd < 0){return FAIL;}

    /* -- send flag 'GET' --*/
    if(!sendOpCode(sockfd,(uint8_t)GET)){return handleErr(sockfd);}
    /* -- send ID length and ID after -- */
    if(!sendIDAndLength(sockfd, ID, 1)){return handleErr(sockfd);}
    if(!sendIDAndLength(sockfd, ID, 2)){return handleErr(sockfd);}
    /* -- send secret -- */
    if(!sendSecret(sockfd, secret)){return handleErr(sockfd);}

    /* -- await response -- */
    uint8_t response = receiveResponse(sockfd);
    if(response != SUCCESS){
        syslog(LOG_ERR,"[-]getBlock() failed to receiveResponse(). Response code = %d\n", response);
        close(sockfd);return response;
    }
    uint32_t data_length = 0;
    if(!receiveDataLength(sockfd, &data_length)){return handleErr(sockfd);}
    if(data_length > buffer_size){
        syslog(LOG_ERR,"[-]getBlock() buffer allocation incorrect (%u vs %u\n", buffer_size, data_length);
        return handleErr(sockfd);
    }
    
    if(!receiveActualData(sockfd,buffer,data_length)){return handleErr(sockfd);}
    close(sockfd);
    return SUCCESS;

}
bool sendBeginText(int fd, uint32_t offset){
    if(!sendBufferForPartial(fd, &offset, sizeof(offset),0)){
        syslog(LOG_ERR,"[-]sendBeginText() failed to sent positioning offset\n");
        return false;
    }
    return true;
}

bool sendBufferForPartial(int fd, void *buff, size_t len){
    size_t total_sent = 0;ssize_t sent = 0;
    while(total_sent < len){
        sent = send(fd,buff + total_sent, len - total_sent, 0);
        if(sent == -1){
            syslog(LOG_ERR,"[-]sendBufferForPartial failed to send buffer data\n");
            return false;
        }
        total_sent += sent;
    }
    return true;
}
bool sendLenText(int fd, uint32_t req_len){
    if(!sendBufferForPartial(fd, &req_len, sizeof(req_len), 0)){
        syslog(LOG_ERR,"[-]sendLenText() failed to send requested length of partial read\n");
        return false;
    }
    return true;
}
uint8_t partRead(char *ID, uint8_t *secret, uint8_t **bufferAccess, uint32_t begin_text, uint32_t length_text){
    /*
    1. Get ID and secret of block to partially read 
    2. Get starting position of text, and the length we want it to end at
    3. Pass in buffer as double pointer

    3.a. Pass in buffer address
    3.b. we malloc buffer - allocate it memory dynamically
    3.c. we then store it back in *buffer
    3.d. allows us to directly modify buffer inside func instead of reallocating oustside as a return val
    */
    int partialfd = connectDaemon();if(partialfd < 0){return FAIL;}
    if(!sendOpCode(partialfd, (uint8_t)GET)){return handleErr(partialfd);}
    if(!sendIDandLength(partialfd, ID, 1)){return handleErr(partialfd);}
    if(!sendIDandLength(partialfd, ID, 2)){return handleErr(partialfd);}
    if(!sendSecret(partialfd, secret)){return handleErr(partialfd);}
    if(!sendBeginText(partialfd, begin_text)){return handleErr(partialfd);}
    if(!sendLenText(fd, len_text)){return handleErr(partialfd);}
    uint8_t read_response = 0;
    if(recv(partialfd, &read_response, sizeof(read_response),0 ) != sizeof(read_response)){
        syslog(LOG_ERR,"[-]partRead() failed to receive response\n");
        return handleErr(partial fd);
    }
    uint32_t recv_length = 0;
    if(!receiveDatalength(partialfd, &recv_length)){return handleErr(fd);}
    *bufferAccess = malloc(recv_length);
    if((*bufferAccess) == 0){
        syslog(LOG_ERR,"[-]partRead malloc for **buffer failed\n");
        close(partialfd);return FAIL;
    }
    if(!receiveActualData(partialfd, *bufferAccess, recv_length)){
        free(*bufferAccess);return handleErr(partialfd);
    }



}

