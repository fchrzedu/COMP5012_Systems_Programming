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
#include <stdbool.h>

#define SEND 0
#define GET 1

#define SUCCESS 0
#define FAIL 1
#define ACCESS_DENIED 2
#define NOT_FOUND 3
#define ALREADY_EXISTS 4
#define SOCKET_PATH "/tmp/domainsocket"
#define MAX_STORAGE 50  // Max number of blocks to stor
/* ----- DATA STORAGE ----- */
typedef struct DataBlock{
    char ID[256];
    uint8_t secret[16];
    uint8_t *data;
    uint32_t data_length;
    int used;

    struct DataBlock *next;
}DataBlock;

DataBlock *head = NULL;
static DataBlock storage[MAX_STORAGE];

// DEBUGGIN HERE !!
void logDebugData(const char *id, uint32_t data_length) {
    FILE *fp = fopen("/tmp/daemon_debug.log", "a");
    if (fp == NULL) {
        syslog(LOG_ERR, "[-] Failed to open debug log file\n");
        return;
    }

    // Find the matching block in memory (linked list)
    DataBlock *b = head;
    while (b != NULL) {
        if (strncmp(b->ID, id, sizeof(b->ID)) == 0) {
            break;
        }
        b = b->next;
    }

    if (b == NULL || b->data == NULL) {
        fprintf(fp, "[-] logDebugData(): No matching data block found for ID='%s'\n", id);
        fclose(fp);
        return;
    }

    // Start logging
    fprintf(fp, "\n[+] Stored Block Info:\n");
    fprintf(fp, "    ID         : %s\n", b->ID);
    fprintf(fp, "    Data Length: %u bytes\n", b->data_length);

    // Log secret in hex
    fprintf(fp, "    Secret     : ");
    for (int i = 0; i < 16; ++i) {
        fprintf(fp, "%02X", b->secret[i]);
        if (i < 15) fprintf(fp, ":");
    }
    fprintf(fp, "\n");

    // Log data as hex for safety (avoid printing raw binary)
    fprintf(fp, "    Data       : ");
    for (uint32_t i = 0; i < b->data_length; ++i) {
        fprintf(fp, "%02X", b->data[i]);
        if ((i + 1) % 16 == 0) fprintf(fp, "\n                  "); // Align next row
        else if (i < b->data_length - 1) fprintf(fp, " ");
    }
    fprintf(fp, "\n");

    fclose(fp);
}


/* ---------- DAEMON CLEANUP - DELETED /TMP/ ----------*/
void cleanup(int server_sock) {
    close(server_sock);
    unlink(SOCKET_PATH);
    closelog();
    syslog(LOG_NOTICE, "[+] Daemon shutdown");
    exit(EXIT_SUCCESS);
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
    listen(server_sock, 2); // Start listening
}

uint8_t respond(int fd, uint8_t r){
    send(fd,&r,sizeof(r),0);
    return r;
}

uint8_t handleSendBlock(int client_sock){
    /*
    2. send ID 
    3. send secret
    4. send data length
    5. send data
    */

    /* - received ID length -*/
    uint8_t id_len = 0;
    if(recv(client_sock, &id_len,sizeof(id_len),0) != sizeof(id_len)){
        syslog(LOG_ERR,"[-]handleSendBlock() failed to receive id length\n");
        return respond(client_sock, FAIL);
    }

    /* - received ID -*/
    char idbuffer[256] = {0};
    if(recv(client_sock, idbuffer, sizeof(idbuffer), 0) != sizeof(idbuffer)){
        syslog(LOG_ERR,"[-]handleSendBlock() failed to receive ID\n");
        return respond(client_sock,FAIL);
    }
    /* - check duplicate ID -*/
    DataBlock *currentptr = head;
    while(currentptr != NULL){
        if (strncmp(currentptr->ID, idbuffer, sizeof(currentptr->ID)) == 0) {
            syslog(LOG_ERR,"[-]handleSendBlock() duplicate ID\n");
            return respond(client_sock,FAIL);
        }
        currentptr = currentptr->next;

    }
    /* - receive secret - */
    uint8_t secret[16] = {0};
    if(recv(client_sock, secret, 16, 0) != 16){
        syslog(LOG_ERR,"[-]handleSendBlock() failed to receive secret\n");
        return respond(client_sock,FAIL);
    }
    /* - receive data length - */
    uint32_t data_length = 0;
    if(recv(client_sock, &data_length, sizeof(data_length), 0) != sizeof(data_length)){
        syslog(LOG_ERR,"[-]handleSendBlock() failed to receive data_length\n");
        return respond(client_sock,FAIL);
    }
    /* - receive data - */
    uint8_t *data = malloc(data_length);
    if(!data){
        syslog(LOG_ERR,"[-]handleSendBlock() failed to malloc for data_length\n");
        return respond(client_sock,FAIL);
    }

    ssize_t totalread = 0;
    while(totalread < data_length){
        ssize_t received = recv(client_sock, data + totalread, data_length - totalread, 0);
        if(received < 1){
            syslog(LOG_ERR,"[-]handleSendBlock() failed to read all of data\n");
            free(data);
            return respond(client_sock,FAIL);
        }
        totalread += received;
    }
    /* - allocate linked pointer storage block -*/
    DataBlock *b = NULL;
    bool isused = false;
    for(int i = 0; i < MAX_STORAGE; i++){
        if(!storage[i].used && !isused){
            b = &storage[i];
            b->used = 1; 
            isused = true;         
        }
    }
    if(!b){
        syslog(LOG_ERR,"[-]handleSendBlock() no storage available\n");
        return respond(client_sock,FAIL);
    }
    /* - populate data struct and link to next head -*/
    strncpy(b->ID,idbuffer,sizeof(b->ID)-1);
    memcpy(b->secret,secret,16);
    b->data = data;
    b->data_length = data_length;
    b->next = head;
    head = b;

    logDebugData(b->ID,b->data_length);

    
    return respond(client_sock, SUCCESS);
    


}

uint8_t handleGetBlock(int client_sock){
    /*
    1. send op code - connectionHandling()
    2. recv Id length
    3. recv ID
    4. recv secret 
    5. send response
    */
    /* -- recv id length -- */

    uint8_t id_len = 0;
    if(recv(client_sock, &id_len,sizeof(id_len),0) != sizeof(id_len)){
        syslog(LOG_ERR,"[-]handleGetBlock() failed to receive ID length\n");
        return respond(client_sock, FAIL);
    }
    /* -- recv id -- */
    char idbuffer[256] = {0};
    if(recv(client_sock, idbuffer, sizeof(idbuffer), 0) != sizeof(idbuffer)){
        syslog(LOG_ERR,"[-]handleGetBlock() failed to receive ID\n");
        return respond(client_sock,FAIL);
    }
    /* -- receive secret -- */
    uint8_t secret[16] = {0};
    if(recv(client_sock, secret, 16, 0) != 16){
        syslog(LOG_ERR,"[-]handleGetBlock() failed to receive secret\n");
        return respond(client_sock,FAIL);
    }
    /* --  compare ID and secret -- */
    DataBlock *b = head;
    while(b!=NULL){
        if(strncmp(b->ID,idbuffer,sizeof(b->ID)) == 0){
            if(memcmp(b->secret, secret, 16) != 0){
                syslog(LOG_ERR,"[-]handleGetBlock() secret mistmatch for ID: %s\n",idbuffer);
                return respond(client_sock, ACCESS_DENIED);
            }
            respond(client_sock,SUCCESS);

            /* -- send data length -- */
            if (send(client_sock, &b->data_length, sizeof(b->data_length), 0) != sizeof(b->data_length)) {
                syslog(LOG_ERR,"[-]handleGetBlock() failed to send data_length\n");
                return FAIL;
            }
            /* -- send data --*/
            ssize_t total_sent = 0;
            while(total_sent < b->data_length){
                ssize_t sent = send(client_sock, b->data + total_sent, b->data_length - total_sent,0);
                if(sent < 1){
                    syslog(LOG_ERR,"[-]handleGetBlock() failed to send data\n");
                    return FAIL;
                }
                total_sent += sent;
            }
            return SUCCESS;                
        }
        b = b->next;        
    }
    syslog(LOG_ERR,"[-]handleGetBlock() no block exists for ID: %s",idbuffer);
    return respond(client_sock, NOT_FOUND);
    

}
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
            continue;
            //exit(EXIT_FAILURE); // change ?? -----------------

        }

        switch(code){
            case SEND:
                resp = handleSendBlock(client_sock);
                break;
            case GET:
                resp = handleGetBlock(client_sock);
                break;
            default:
                syslog(LOG_ERR,"[-] Unknown opcode received: %d",code);
                send(client_sock, &(uint8_t){FAIL}, sizeof(uint8_t),0);
                break;                
        }
        close(client_sock);

        
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
    openlog("*DAEMON*", LOG_PID, LOG_DAEMON);
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