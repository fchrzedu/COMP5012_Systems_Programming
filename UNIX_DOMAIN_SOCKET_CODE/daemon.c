#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SEND_BLOCK 0
#define GET_BLOCK 1

#define SUCCESS_RES 0
#define FAIL_RES 1
#define ACCESS_DENIED_RES 2
#define NOT_FOUND_RES 3
#define ALREADY_EXISTS_RED 4

#define SOCKET_PATH "/tmp/unixdomainsocket"
#define MAX_STORAGE 100  // Max number of blocks to store

/* ---------- DATA STORAGE ----------*/
typedef struct {
    char ID[256]; //ident of block
    uint8_t secret[16]; // 16byte secret password
    uint8_t *data; // ptr to actual data stored
    uint32_t data_length; // length of data
    int is_used; // flag to check whether data block is being used (1 yes | 0 no )
} DataBlock;

static DataBlock storage[MAX_STORAGE];
/* ---------- DAEMON CLEANUP - DELETED /TMP/ ----------*/
void cleanup(int server_sock) {
    close(server_sock);
    unlink(SOCKET_PATH);
    closelog();
    syslog(LOG_NOTICE, "[+] Daemon shutdown");
    exit(EXIT_SUCCESS);
}
/* ---------- FIND BLOCK USING ID ----------*/
int findIndxById(const char * id){
    for(int i = 0; i < MAX_STORAGE; i++){
        if(storage[i].is_used && strncmp(storage[i].ID,id,256) == 0){
            return i;
        }
    }
    return -1;
}

/* ---------- FIND AVALIABLE STORAGE INDEX ----------*/
int findFreeBlockIndx(){
    for(int i = 0; i < MAX_STORAGE; i++){
        if(!storage[i].is_used){
            return i;
        }
    }
    return -1;
}

/* ---------- CREATES SERVER SOCKET ----------*/

int initSocket() {
    int server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        syslog(LOG_ERR, "[-] Server socket creation failed!\n");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_NOTICE, "[+] socket() done");
    return server_sock;
}

void pathLogSetup(char *buff, size_t buff_size) {
    memset(buff, 0, buff_size); // Fill buffer with 0s
    strncpy(buff, SOCKET_PATH, buff_size - 1); // Copy the path
    syslog(LOG_NOTICE, "testbuf = %s", buff);
}
/* ---------- SERVER ADDRESS SETUP ----------*/
void serverSetup(struct sockaddr_un *server_address) {
    memset(server_address, 0, sizeof(struct sockaddr_un)); // Clear struct
    (*server_address).sun_family = AF_UNIX; // Set socket family
    strncpy((*server_address).sun_path, SOCKET_PATH, sizeof((*server_address).sun_path) - 1); // Copy path
    syslog(LOG_NOTICE, "[+] Memset worked");
}
/* ---------- BIND() & LISTEN() ----------*/
void bindListen(int server_sock, struct sockaddr_un *server_address) {
    unlink(SOCKET_PATH); // Remove any old socket file

    if (bind(server_sock, (struct sockaddr *)server_address, sizeof(struct sockaddr_un)) == -1) {
        syslog(LOG_ERR, "[-] Bind() error\n");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    syslog(LOG_NOTICE, "[+] Bind() success\n");
    listen(server_sock, 1); // Start listening
}


/* ---------- HANDLES libshare::sendNewBlock() ----------*/
uint8_t handleSendBlock(int clientfd){
    char id[256] = {0};
    uint8_t secret[16];
    uint32_t data_length;
    uint8_t *data = NULL;
    ssize_t recvd;
    uint8_t resp = FAIL_RES;

    /* Read secret, ID, data_length*/

    /* ---------- ID HANDLING ----------*/
    recvd = recv(clientfd,id,sizeof(id),0);
    if(recvd!=sizeof(id)){
        syslog(LOG_ERR,"[-] failed to read id from client\n");        
        send(clientfd,&resp,sizeof(resp),0);
        return resp;      
    }   
    syslog(LOG_ERR,"DEBUG ID: %5s",id);
    
    /* ---------- SECRET HANDLING ----------*/
    recvd =recv(clientfd, secret, sizeof(secret), 0);
    if( recvd != sizeof(secret)){
        syslog(LOG_ERR,"[-] failed to receive secret from client\n");        
        send(clientfd,&resp,sizeof(resp),0);
        return resp;
    }   
 
    
    /* ---------- DATA_LENGTH HANDLING ----------*/
    recvd = recv(clientfd, &data_length, sizeof(data_length),0);
    if(recvd != sizeof(data_length)){
        syslog(LOG_ERR,"[-] failed to receive data_length from client\n");
        send(clientfd,&resp,sizeof(resp),0);
        return resp;
    }        syslog(LOG_ERR,"DEBUG DATALEN: %5u",data_length);

    /* ---------- MALLOC BUFFER ALLOCATING ----------*/
    data = malloc(data_length); /* allocate buffer as large as data_length */
    if(!data){
        syslog(LOG_ERR,"[-] malloc buffer allocation failed\n");
        send(clientfd,&resp,sizeof(resp),0);
        return resp;
    }

    /* ---------- RECIEVING DATA ALLOCATING ----------*/
    syslog(LOG_ERR,"receiving data......\n");
    ssize_t total_received = 0;
    while(total_received < data_length){
        recvd = recv(clientfd, data + total_received, data_length - total_received,0);
        if(recvd < 1){
            syslog(LOG_ERR,"[-] failed to receive block data\n");
            free(data);
            send(clientfd,&resp,sizeof(resp),0);
            return resp;
        }
        total_received += recvd;
    }        
    
    /* ---------- FIND DUPLICATE BLOCK ----------*/
    if(findIndxById(id) != -1){
        syslog(LOG_ERR,"[-] duplicate block ID: %s",id);
        free(data);
        resp = ALREADY_EXISTS_RED;
        send(clientfd,&resp,sizeof(resp),0);
        return resp;
    }
    /* ---------- FIND FREE STORAGE[] BLOCK ----------*/
    int indx = findFreeBlockIndx();
    if(indx == -1){
        syslog(LOG_ERR,"[-] failed to store block. full storage\n");
        free(data);
        resp = FAIL_RES;
        send(clientfd,&resp,sizeof(resp),0);
        return resp;
    }
    /* ---------- STORE BLOCK IN STORAGE[] ----------*/
    strncpy(storage[indx].ID,id,sizeof(storage[indx].ID) -1 );
    memcpy(storage[indx].secret,secret,sizeof(secret));
    storage[indx].data = data;
    storage[indx].data_length = data_length;
    storage[indx].is_used = 1;

    

    resp = SUCCESS_RES;
    send(clientfd,&resp,sizeof(resp),0);
    
    free(data);
    
    return resp;    
}

/* ---------- HANDLES libshare::getBlock() ----------*/


/* ---------- CLIENT CONNECT() FROM LIB WORKS ----------*/
void connectionHandling(int server_sock) {
    int client_sock;
    struct sockaddr_un client_address;
    socklen_t client_len;
    uint8_t code; /* OPCODE */
    uint8_t resp; /* STORED RESPONSE FLAG*/

    while (1) {
        client_len = sizeof(client_address);
        client_sock = accept(server_sock, (struct sockaddr *)&client_address, &client_len);

        if (client_sock < 0) {
            syslog(LOG_ERR, "[-]Accept() error\n");
            close(client_sock);
            exit(EXIT_FAILURE);
        }
        syslog(LOG_NOTICE, "[+]Accept() success\n");

        /* READ OPCODE FROM SHARELIB */
        if(recv(client_sock, &code, sizeof(code), 0) != sizeof(code)){
            syslog(LOG_ERR,"[-] failed to read opcode from sharelib\n");
            close(client_sock);
            exit(EXIT_FAILURE); // change ?? -----------------

        }

        switch(code){
            case SEND_BLOCK:
                resp = handleSendBlock(client_sock);
                break;
            case GET_BLOCK:
                //resp = handleGetBlock(client_sock);
                break;
            default:
            /* directly casts FAIL_RES, rather than uint8_t response = FAIL_RES; */
                syslog(LOG_ERR,"[-] Unknown opcode received: %d",code);
                send(client_sock, &(uint8_t){FAIL_RES}, sizeof(uint8_t),0);
                break;
        }
    }
        
        
}

/* ---------- DAEMONIZES ----------*/
void daemonize() {
    pid_t pid, sid;

    pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "[-] Forking error");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // Parent exits
    }

    sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, "[-] setsid error");
        exit(EXIT_FAILURE);
    }

    if (chdir("/") < 0) {
        syslog(LOG_ERR, "[-] chdir error");
        exit(EXIT_FAILURE);
    }

    umask(0);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    syslog(LOG_NOTICE, "[+] Daemon initialized and detached");
}

int main() {
    printf("About to daemonsize\n");
    openlog(">>DAEMONSOCKETSERV", LOG_PID, LOG_DAEMON);
    daemonize();

    int server_sock = initSocket();
    struct sockaddr_un server_address;
    char buff[108]; // Exact size of sun_path

    pathLogSetup(buff, sizeof(buff));
    serverSetup(&server_address);
    bindListen(server_sock, &server_address);
    connectionHandling(server_sock);
    cleanup(server_sock);

    return 0;
}