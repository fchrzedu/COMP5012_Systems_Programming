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
/* operation codes */
#define SEND 0
#define GET 1
#define PARTIAL_GET 2
#define UPDATE 3
/* REPONSE CODES */
#define SUCCESS 0
#define CLOSE 5
#define FAIL 1
#define ACCESS_DENIED 2
#define NOT_FOUND 3
#define ALREADY_EXISTS 4

/* CONSTANTS */
#define SOCKET_PATH "/tmp/domainsocket"
#define MAX_STORAGE 50  // Max number of blocks to store

/* ----- PERMISSION AND SECRETS (un-implemented, left for future as template) -----*/
typedef struct Permission{
    uint8_t secret[16];
    int perm;
    struct Permission *next;
} Permission;
/* ----- DATA STORAGE BLOCK ----- */
typedef struct DataBlock{
    char ID[256];
    uint8_t secret[16];
    uint8_t *data;
    uint32_t data_length;
    int used;
    //time_t updated_time;
    //Permission *Permissions;

    struct DataBlock *next;
}DataBlock;

DataBlock *head = NULL; /* global head pointer variable */
static DataBlock storage[MAX_STORAGE];



/* ---------- DAEMON CLEANUP - DELETED /TMP/ ----------*/
void cleanup(int server_sock) {
    close(server_sock);
    unlink(SOCKET_PATH);

    // Free all DataBlocks in the linked list
    DataBlock *current = head;
    while (current != NULL) {
        DataBlock *next = current->next;
        free(current->data); // Free the data pointer
        current->data = NULL;
        current->next = NULL;
        current = next;
    }
    head = NULL;

    syslog(LOG_NOTICE, "[+] Daemon shutdown and memory freed\n");
    closelog();
    exit(EXIT_SUCCESS);
}
/* ---------- CREATES SERVER SOCKET ----------*/
int initSocket() {
    int server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        syslog(LOG_ERR, "[-] initSocket() server socket creation failed!\n");
        exit(EXIT_FAILURE);
    }
    syslog(LOG_NOTICE, "[+] socket() done");
    return server_sock;
}
/* ---------- DAEMONIZES ----------*/
void daemonize() {
    pid_t pid, sid;
    /* forks current session, deattaches itself from terminal & std(in/out) redirecting to root */
    pid = fork();
    if (pid < 0) {
        syslog(LOG_ERR, "[-] fork() error");
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS); // Parent exits
    }

    sid = setsid();
    if (sid < 0) {
        syslog(LOG_ERR, "[-] setsid() error");
        exit(EXIT_FAILURE);
    }

    if (chdir("/") < 0) {
        syslog(LOG_ERR, "[-] chdir() error");
        exit(EXIT_FAILURE);
    }

    umask(0);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    syslog(LOG_NOTICE, "[+] Daemon initialized and detached");
}
/* ---------- COPIES SOCKET_PATH TO SERVER  ----------*/
void pathLogSetup(char *buff, size_t buff_size) {    
    memset(buff, 0, buff_size);
    strncpy(buff, SOCKET_PATH, buff_size - 1); 
}
/* ---------- SERVER ADDRESS SETUP ----------*/
void serverSetup(struct sockaddr_un *server_address) {
    /* 
    clears server structure (pre-caution), sets it to AF_UNIX socket family
    Copies the server sun path address 
    */
    memset(server_address, 0, sizeof(struct sockaddr_un)); // Clear struct
    (*server_address).sun_family = AF_UNIX; // Set socket family
    strncpy((*server_address).sun_path, SOCKET_PATH, sizeof((*server_address).sun_path) - 1); // Copy path
    syslog(LOG_NOTICE, "[+] Memset() worked");
}
/* ---------- BIND() & LISTEN() ----------*/
void bindListen(int server_sock, struct sockaddr_un *server_address) {
    /* removes any existing socket /tmp/ files - proceeds to bind to server and listens for maximum 10 clients */
    unlink(SOCKET_PATH); 
    if (bind(server_sock, (struct sockaddr *)server_address, sizeof(struct sockaddr_un)) == -1) {
        syslog(LOG_ERR, "[-]bindListen() Bind() error\n");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    syslog(LOG_NOTICE, "[+] Bind() success\n");
    listen(server_sock, 10); 
}
/* ---------- HELPER: CLIENT RESPONSE CODE ----------*/
uint8_t respond(int fd, uint8_t r){
    /* -- sends response to client --*/
    send(fd,&r,sizeof(r),0);
    return r;
}
/* ---------- HELPER: RECEIVES ALL DATA ----------*/
bool receiveAll(int sfd, void *b, size_t len){
    /*
    loops through data length
    increments counter and matches whether == data length
    (sometimes send() may not send all data at once, but rather in chunks) 
    */
    size_t tot = 0; ssize_t received = 0;
    while(tot<len){
        received = recv(sfd, (char*)b + tot, len - tot, 0);
        if(received <1 ){
            return false;
        }
        tot+=received;
    }
    return true;
}
/* ---------- HELPER: SENDS ALL DATA ----------*/
bool sendAll(int sfd, const void *b, size_t len){
    /*
    loops through data length
    increments counter and matches whether == data length
    (sometimes recv() may not receive all data at once, but rather in chunks) 
    */
    size_t tot = 0; ssize_t sent = 0;
    while(tot < len){
        sent = send(sfd, (const char*)b + tot, len - tot, 0);
        if(sent <1){return false;}
        tot+=sent;
    }
    return true;
}
/* ---------- HELPER FUNCTIONS CHECKING ALL DATA ----------*/
bool isValid(const char *id, uint8_t id_len, const uint8_t *sec, uint32_t data_len,const uint8_t *data ){
    /* check whether ID & it's length are within bounds. Same for secret, data and data length*/
    if(id_len < 0 || id_len > 255 ){
        syslog(LOG_ERR,"[-]isValid(): ID length is invalid (length of %u)",id_len);
        return false;
    }
    for(uint8_t i = 0; i<id_len;i++){
        /* checks whether any of ID is null terminates EXCEPT the actual last index which is \0 */
        if(id[i] == '\0'){
            syslog(LOG_ERR,"[-]isValid(): ID contains zero null byte at position: %u", i);
            return false;
        }
    }
    if(sec == NULL){
        syslog(LOG_ERR,"[-]isValid(): no hexadecimal values stored in secret\n");
        return false;
    }
    if(data_len == 0){
        syslog(LOG_ERR,"[-]isValid(): data length is invalid of size: %u",data_len);
        return false;
    }
    if(data == NULL){
        syslog(LOG_ERR,"[-]isValid(): data is empty (NULL");
        return false;
    }
    return true;

}
/* ---------- HANDLES lib.c::sendNewBlock()*/
uint8_t handleSendBlock(int client_sock){
    /* - get ID and its length - */
    uint8_t id_len = 0;    
    if(!receiveAll(client_sock, &id_len, sizeof(id_len))){
        syslog(LOG_ERR,"[-]handleSendBlock() failed to receive id length\n");
        return respond(client_sock, FAIL);
    }        
    char idbuffer[256] = {0};
    if(!receiveAll(client_sock, idbuffer, id_len)){
        syslog(LOG_ERR,"[-]handleSendBlock() failed to receive ID\n");
        return respond(client_sock,FAIL);
    }
    idbuffer[id_len] = '\0'; // null terminate end of string

    /* check duplicate ID for overwriting, same for secret*/
    DataBlock *currentptr = head;
    while(currentptr != NULL){
        /* compare stored ID passed in ID . == 0 means no inconcistencies */
        if (strncmp(currentptr->ID, idbuffer, sizeof(currentptr->ID)) == 0) {
            uint8_t secret[16] = {0};
            if(!receiveAll(client_sock, secret, 16)){
                syslog(LOG_ERR,"[-]handleSendBlock() failed to receive secret\n");
                return respond(client_sock,FAIL);
            }
            /* compares input and stored secret byte by byte*/
            if(memcmp(currentptr->secret, secret, 16) == 0){
                syslog(LOG_NOTICE,"[!]handleSendBlock() duplicate ID & secret - please overwrite \n");
                return respond(client_sock, ALREADY_EXISTS);
            }
            else{
                syslog(LOG_ERR,"[!]handleSendBlock() matching ID, mismatching secret\n");
                return respond(client_sock, ACCESS_DENIED);
            }           
        }
        currentptr = currentptr->next;

    }
    /* - receive secret - */
    uint8_t secret[16] = {0};
    if(!receiveAll(client_sock,secret,16)){
        syslog(LOG_ERR,"[-]handleSendBlock() failed to receive secret\n");
        return respond(client_sock,FAIL);
    }
    
    /* - receive data length - */
    uint32_t data_length = 0;
    if(!receiveAll(client_sock,&data_length,sizeof(data_length))){
        syslog(LOG_ERR,"[-]handleSendBlock() failed to receive data_length\n");
        return respond(client_sock,FAIL);
    }
    
    /* - allocate buffer for size of data, then receive data and store - */
    uint8_t *data = malloc(data_length);
    if(!data){
        syslog(LOG_ERR,"[-]handleSendBlock() failed to malloc for data_length\n");
        return respond(client_sock,FAIL);
    }

    if(!receiveAll(client_sock, data, data_length)){
        syslog(LOG_ERR,"[-]handleSendBlock() failed to read all of data\n");
        free(data);
        return respond(client_sock,FAIL);
    }
    /* - check whether all input arguments are valid (extra precaution considering lib.c client check helper methods) - */
    if(!isValid(idbuffer, id_len, secret, data_length, data)){free(data);return respond(client_sock, FAIL);}
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
    return respond(client_sock, SUCCESS);

}
/* ----------- HANDLES lib.c::getBlock() ----------*/
uint8_t handleGetBlock(int client_sock){ 
    /* - recv ID length - */
    uint8_t id_len = 0;
    if(!receiveAll(client_sock, &id_len, sizeof(id_len))){
        syslog(LOG_ERR,"[-]handleGetBlock() failed to receive ID length\n");
        return respond(client_sock, FAIL);
    }        
    /* -- recv ID -- */
    char idbuffer[256] = {0};
    if(!receiveAll(client_sock, idbuffer, id_len)){
        syslog(LOG_ERR,"[-]handleGetBlock() failed to receive ID\n");
        return respond(client_sock,FAIL);
    }
    idbuffer[id_len] = '\0';      
    /* -- receive secret -- */
    uint8_t secret[16] = {0};
    if(!receiveAll(client_sock, secret, 16)){
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
            if(respond(client_sock,SUCCESS) != SUCCESS){
                syslog(LOG_ERR,"[-]handleGetBlock() failed to send SUCCESS OPCODE");
                return FAIL;
            }

            /* -- send data length -- */
            if(!sendAll(client_sock, &b->data_length,sizeof(b->data_length))){
                syslog(LOG_ERR,"[-]handleGetBlock() failed to send data_length\n");
                return FAIL;
            }            
            /* -- send data --*/
            if(!sendAll(client_sock, b->data, b->data_length)){
                syslog(LOG_ERR,"[-]handleGetBlock() failed to send data\n");
                return FAIL;
            }            
            return SUCCESS;                
        }
        b = b->next;        
    }
    syslog(LOG_ERR,"[-]handleGetBlock() no block exists for ID: %s",idbuffer);
    return respond(client_sock, NOT_FOUND);
    

}
/* ---------- HANDLES lib.c::getPartialBlock() ----------*/
uint8_t handlePartialGetBlock(int client_sock){
    uint8_t id_len = 0;
    char idbuff[256] = {0}; 
    uint8_t secret[16] = {0};
    uint32_t begin_text_offset = 0;
    uint32_t len_text = 0;
    /* - receive ID length and ID - */
    if(!receiveAll(client_sock, &id_len, sizeof(id_len))){
        syslog(LOG_ERR, "[-]handlePartialGetBlock() failed to receive ID length\n");
        return respond(client_sock, FAIL);
    }
    if(!receiveAll(client_sock, idbuff, id_len)){
        syslog(LOG_ERR,"[-]handlePartialGetBlock() failed to receive ID tag\n");
        return respond(client_sock, FAIL);
    } 
    /* - receive secret - */
    if(!receiveAll(client_sock, secret, 16)){
        syslog(LOG_ERR,"[-]handlePartialGetBlock() failed to receive secret\n");
        return respond(client_sock, FAIL);
    }
    /* - receive starting offset & length - */
    if(!receiveAll(client_sock, &begin_text_offset, sizeof(begin_text_offset))){
        syslog(LOG_ERR,"[-]handlePartialGetBlock() failed to receive begin_text_offset\n");
        return respond(client_sock, FAIL);
    }
    if(!receiveAll(client_sock, &len_text, sizeof(len_text))){
        syslog(LOG_ERR,"[-]handlePartialGetBlock() failed to receive length needed for offset\n");
        return respond(client_sock, FAIL);
    }

    // search for block using ID and secret
    DataBlock *block = head;
    while(block != NULL){
        if(strncmp(block->ID, idbuff, sizeof(block->ID)) == 0){
            if(memcmp(block->secret, secret, 16) != 0){
                syslog(LOG_ERR,"[-]handlePartialGetBlock() secrets not matching\n");
                return respond(client_sock, ACCESS_DENIED);
            }
            // check whether offset and length are valid
            if(begin_text_offset >= block->data_length){
                syslog(LOG_ERR,"[-]handlePartialGetBlock() offset > block's data length\n");
                return respond(client_sock, FAIL);
            }
            //if length is greater, adjust it
            if(begin_text_offset + len_text > block->data_length){
                len_text = block->data_length - begin_text_offset;
            }

            // send success response
            if(respond(client_sock, SUCCESS) != SUCCESS){
                syslog(LOG_ERR,"[-]handlePartialGetBlock() failed to respond 'SUCCESS\n");
                return FAIL;
            }
            //send data after adjusting its length
            if(!sendAll(client_sock, &len_text, sizeof(len_text))){
                syslog(LOG_ERR,"[-]handlePartialGetBlock() fail sending snipped length\n");
                return FAIL;
            }

            if(!sendAll(client_sock, block->data + begin_text_offset, len_text)){
                syslog(LOG_ERR,"[-]handlePartialGetBlock() failed sending snipped data\n");
                return FAIL;
            }
            return SUCCESS;
        }
        block = block->next;
    }
    syslog(LOG_ERR,"[-]handlePartialGetBlock() ID:%s not found\n",idbuff);
    return respond(client_sock, NOT_FOUND);   

}
/* ---------- HANDLES lib.c::overwriteBlock() ---------*/
uint8_t handleOverwriteBlock(int client_sock){
    uint8_t id_len = 0;
    char idbuffer[256] = {0};
    uint8_t secret[16] = {0};
    uint32_t n_data_len = 0; /* different 'data' = different size*/
    /* - receive ID length with ID - */
    if(!receiveAll(client_sock, &id_len, sizeof(id_len))){
        syslog(LOG_ERR,"[-]handleOverwriteBlock() failed to receive ID length\n");
        return respond(client_sock, FAIL);
    }

    if(!receiveAll(client_sock, idbuffer, id_len)){
        syslog(LOG_ERR,"[-]handleOverwriteBlock() failed to receive ID\n");
        return respond(client_sock, FAIL);
    }
    /* - receive secret - */
    if(!receiveAll(client_sock, secret, 16)){
        syslog(LOG_ERR,"[-]handleOverwriteBlock() failed to receive secret\n");
        return respond(client_sock, FAIL);
    }
    /* - receive data length followed by data- */
    if (!receiveAll(client_sock, &n_data_len, sizeof(n_data_len))) {
        syslog(LOG_ERR, "[-]handleOverwriteBlock() failed to receive new data length\n");
        return respond(client_sock, FAIL);
    }
    /* - allocate new data buffer, free old - */
    uint8_t *n_data = malloc(n_data_len);
    if(!n_data){
        syslog(LOG_ERR, "[-]handleOverwriteBlock() failed to malloc mem for data\n");
        return respond(client_sock, FAIL);
    }
    if (!receiveAll(client_sock, n_data, n_data_len)) {
        syslog(LOG_ERR, "[-]handleOverwriteBlock() failed to receive new data \n");
        return respond(client_sock, FAIL);
    }
    /* - valiate inputs - */
    if (!isValid(idbuffer, id_len, secret, n_data_len, n_data)) {
        free(n_data);
        return respond(client_sock, FAIL);
    }

    /* - iterate through all variables, check whether ID's and secrets match - */
    DataBlock *block = head;
    while(block != NULL){
        if(strncmp(block->ID, idbuffer, sizeof(block->ID)) == 0){
            if(memcmp(block->secret, secret, 16) != 0){
                syslog(LOG_ERR,"[-]handleOverwriteBlock() different secrets (mismatch)\n");
                free(n_data);
                return respond(client_sock, ACCESS_DENIED);
            }
            /* - if secret == ID, replace old ata with new and free old data -*/
            free(block->data);
            block->data = n_data;
            block->data_length = n_data_len;
            syslog(LOG_NOTICE,"[!]handleOverwriteBlock() overwrite for ID:%s",idbuffer);
            free(n_data);
            return respond(client_sock, SUCCESS);

        }
        block = block->next;
    }
    syslog(LOG_ERR, "[-]handleOverwriteBlock() no block exists for ID: %s\n", idbuffer);
    
    return respond(client_sock, NOT_FOUND);
}

/* ---------- CLIENT CONNECT() FROM LIB WORKS ----------*/
void connectionHandling(int server_sock) {
    /* client socket side setup variables */
    int client_sock;
    struct sockaddr_un client_address;
    socklen_t client_len;
    /* creates variable for daemon response, and lib.c retrieved operation needed */
    uint8_t code; 
    uint8_t resp; 
    /* while(1) - not neccesarily the best while loop, but it stays infinite */
    while (1) {
        /* give client a server and accept the socket connection under client_sock */
        client_len = sizeof(client_address);
        client_sock = accept(server_sock, (struct sockaddr *)&client_address, &client_len);

        if (client_sock < 0) {
            syslog(LOG_ERR, "[-]connectionHandling() Accept() error\n");
            close(client_sock);
            continue; /* needed to keep the daemon listening to other sequential clients */
        }
        syslog(LOG_NOTICE, "[+]connectionHandling() Accept() success\n");

        /* READ OPCODE FROM SHARELIB */
        if(recv(client_sock, &code, sizeof(code), 0) != sizeof(code)){
            syslog(LOG_ERR,"[-] failed to read opcode from sharelib\n");
            close(client_sock);
            continue; 
            /* don't want to close client incase they may send additional opcodes */
            
        }
        /* switch case to handle lib.c operations needed */
        syslog(LOG_NOTICE,"[!] received opcode:%d",code);
        switch(code){
            case SEND:
                resp = handleSendBlock(client_sock);
                break;
            case GET:
                resp = handleGetBlock(client_sock);
                break;
            case PARTIAL_GET:
                resp = handlePartialGetBlock(client_sock);
                break;
            case UPDATE:
                resp = handleOverwriteBlock(client_sock);
                break;
            case CLOSE:
                syslog(LOG_NOTICE,"[!]Closing client (%u) connection\n",client_sock);
                close(client_sock);
                break;
            default:
                syslog(LOG_ERR,"[-] Unknown opcode received: %d",code);
                send(client_sock, &(uint8_t){FAIL}, sizeof(uint8_t),0);
                break;                
        }
        close(client_sock);
        
    }       
}



int main() {
    printf("About to daemonsize, /var/log/messages under 'DAEMON'\n");
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