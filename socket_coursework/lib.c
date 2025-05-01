#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <stdbool.h>
#include <ctype.h>
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
bool sendBeginText(int fd, uint32_t begin_text){
    if(send(fd, &begin_text, sizeof(begin_text), 0) != sizeof(begin_text)){
        syslog(LOG_ERR,"[-]sendBeginText() failed to send offset for partial get\n");
        return false;
    }
    return true;
}
bool sendLengthText(int fd, uint32_t length_text){
    if(send(fd, &length_text, sizeof(length_text),0) != sizeof(length_text)){
        syslog(LOG_ERR,"[-]sendLengthText() failed to send length needed for offset\n");
        return false;
    }return true;
}
bool receivePartData(int fd, void *buff, uint32_t len){
    uint32_t total_read = 0;
    uint32_t data_len = 0;
    if(recv(fd, &data_len, sizeof(data_len),0) != sizeof(data_len)){
        syslog(LOG_ERR,"[-]receivePartData() failed to receive snippet length\n");
        return false;
    }
    if(data_len != len){
        syslog(LOG_ERR,"[-]Received snippet length %u does not match expected length %u\n", data_len, len);
        return false;
    }
    while(total_read < len){
        ssize_t recvD = recv(fd, buff + total_read, len - total_read, 0);
        if(recvD < 1){
            syslog(LOG_ERR,"[-]receivePartData() failed to receive partial data\n");
            return false;
        }
        total_read +=recvD;
    }
    return true;
}

uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data){
    logOpen();
    int sockfd = connectDaemon();
    if(sockfd < 0){
        return FAIL;
    }   
    /* -- send flag 'SEND' -- */
    if(!sendOpCode(sockfd, (uint8_t)SEND)){
        return handleErr(sockfd);
    }
    /* - send ID length followed by ID - */
    if(!sendIDAndLength(sockfd, ID, 1)){
        return handleErr(sockfd);
    }
    if(!sendIDAndLength(sockfd, ID, 2)){
        return handleErr(sockfd);
    }

    /* - send secret - */
    if(!sendSecret(sockfd, secret)){
        return handleErr(sockfd);
    }
    
    /* - send data_length - */
    if(!sendDataLength(sockfd, data_length)){
        return handleErr(sockfd);
    }
    if(!sendActualData(sockfd, data, data_length)){
        return handleErr(sockfd);
    }  

    uint8_t response = receiveResponse(sockfd);   
    if(response == ALREADY_EXISTS){
        syslog(LOG_NOTICE,"[!] Block with ID '%s' already exists.", ID);
        close(sockfd);
        return ALREADY_EXISTS;
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
// removed *begin_text

uint8_t partialGetBlock(char *ID, uint8_t *secret, void **bufferAccess, uint32_t *begin_text, uint32_t length_text){
    /*
    1. Get ID and secret of block to partially read 
    2. Get starting position of text, and the length we want it to end at
    3. Pass in buffer as double pointer

    3.a. Pass in buffer address
    3.b. we malloc buffer - allocate it memory dynamically
    3.c. we then store it back in *buffer
    3.d. allows us to directly modify buffer inside func instead of reallocating oustside as a return val
    */
    logOpen();
    int partfd = connectDaemon();if (partfd < 1){return FAIL;} // send op code
    if(!sendOpCode(partfd, (uint8_t)PARTIAL_GET)){return handleErr(partfd);}
    // send ID & length
    if(!sendIDAndLength(partfd, ID, 1)){return handleErr(partfd);}
    if(!sendIDAndLength(partfd, ID, 2)){return handleErr(partfd);}
    // send secret
    if(!sendSecret(partfd, secret)){return handleErr(partfd);}
    // send where to start in the message
    if(!sendBeginText(partfd, *begin_text)){return handleErr(partfd);}
    // send length from offset
    if(!sendLengthText(partfd, length_text)){return handleErr(partfd);}

    // await daemon response
    uint8_t resp = receiveResponse(partfd);
    if(resp != SUCCESS){
        syslog(LOG_ERR,"[-]partialGetBlock() failed on receiveResponse() error\n");
        close(partfd);return resp;
    }
    // allocate memory for buffer (double ** to access directly)
    if((*bufferAccess = malloc(length_text + 1)) == 0){
        syslog(LOG_ERR,"[-]partialGetBlock() **buffer == 0\n");
        close(partfd);return handleErr(partfd);
    }
    memset(*bufferAccess,0,length_text + 1);
    // receive partial data from daemon
    if(!receivePartData(partfd, *bufferAccess, length_text)){
        free(*bufferAccess);return handleErr(partfd);
    }    
    ((char *)*bufferAccess)[length_text] = '\0';

    close(partfd); return SUCCESS;   

}
uint8_t overwriteBlock(char *ID, uint8_t *secret, uint32_t data_len, void *data){
    int overfd = connectDaemon();
    if(overfd < 0){return FAIL;}

    if(!sendOpCode(overfd, (uint8_t)UPDATE)){return handleErr(overfd);}

    if(!sendIDAndLength(overfd, ID, 1)){return handleErr(overfd);}
    if(!sendIDAndLength(overfd, ID, 2)){return handleErr(overfd);}

    if(!sendSecret(overfd, secret)){return handleErr(overfd);}

    if(!sendDataLength(overfd, data_len)){return handleErr(overfd);}

    if(!sendActualData(overfd, data, data_len)){return handleErr(overfd);}
    uint8_t response = receiveResponse(overfd); close(overfd); return response;

}

void clientMenu() {
    printf("[1] Send new block to server\n");
    printf("[2] Update an existing block's payload\n");
    printf("[3] Retrieve partial payload from a block\n");
    printf("[4] Retrieve a block's whole data payload\n");
    printf("[5] Disconnect from the server\n");
    printf("Enter your choice as number> ");
}

bool isValidSecret(const char *input) {
    int sec_len = strlen(input);
    int counter = 0;

    for (int i = 0; i < sec_len; i++) {
        if (isxdigit(input[i])) {
            // Valid hex digit
            continue;
        } else if (input[i] == ' ') {
            // Valid space between hex pairs
            counter++;
        } else {
            return false; // Invalid character
        }
    }

    // Ensure there are exactly 15 spaces (16 bytes) and no trailing space
    return counter == 15 && input[sec_len - 1] != ' ';
}

void readClientSecret(uint8_t *s) {
    char sec_input[64] = {0}; // Buffer to hold input
    bool valid = false;

    while (!valid) {
        printf("Secret must be 16 bytes long, where each is a 2-character hexadecimal separated by whitespace\n");
        printf("Enter secret> ");
        fgets(sec_input, sizeof(sec_input), stdin);
        sec_input[strcspn(sec_input, "\n")] = '\0'; // Remove newline character

        if (isValidSecret(sec_input)) {
            valid = true; // Input is valid, exit loop
            // Parse the valid secret into the uint8_t array
            char *tok = strtok(sec_input, " ");
            for (int i = 0; i < 16 && tok != NULL; i++) {
                sscanf(tok, "%hhx", &s[i]);
                tok = strtok(NULL, " ");
            }
        } else {
            printf("[-] Invalid secret format. Please try again.\n");
        }
    }
}

void readClientID(char *id, size_t size) {
    printf("Enter ID> ");
    fgets(id, size, stdin);
    id[strcspn(id, "\n")] = '\0'; // Null-terminate ID
}

void readClientData(char *data, size_t size) {
    printf("Enter data> ");
    fgets(data, size, stdin);
    data[strcspn(data, "\n")] = '\0'; // Null-terminate data
}

void handleClientResponse(uint8_t resp, const char *successMsg, const char *failureMsg) {
    if (resp == SUCCESS) {
        printf("[+] %s\n", successMsg);
    } else {
        printf("[-] %s\n", failureMsg);
    }
}
