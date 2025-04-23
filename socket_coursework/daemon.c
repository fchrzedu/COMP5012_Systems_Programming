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
#define SOCKET_PATH "/tmp/daemon_unix_socket"
/* ----- DAEMON CLEANUP - UNLINK(TMP) ----- */
void cleanup(int server_sock) {
    close(server_sock);
    unlink(SOCKET_PATH);
    closelog();
    syslog(LOG_NOTICE, "[+] Daemon shutdown");
    exit(EXIT_SUCCESS);
}
/* ----- DATA STORAGE ----- */
typedef struct {
    char ID[256];
    uint8_t secret[16];
    uint8_t *data;
    uint32_t data_length;
    int used;

    struct DataBlock *next;
}DataBlock;

/* ----- INITIALISES SERVER UNIX SOCKET */
int initSocket() {
    int server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock < 0) {
        syslog(LOG_ERR, "[-]initSocket() server socket creation failed!\n");
        exit(EXIT_FAILURE);
    }
    return server_sock;
}
/* ----- SETS UP SOCKET PATH ----- */
void pathLogSetup(char *buff, size_t buff_size) {
    memset(buff, 0, buff_size); // Fill buffer with 0s
    strncpy(buff, SOCKET_PATH, buff_size - 1); // Copy the path
    syslog(LOG_NOTICE, "testbuf = %s", buff);
}
void serverSetup(struct sockaddr_un *server_address) {
    memset(server_address, 0, sizeof(struct sockaddr_un)); // Clear struct
    (*server_address).sun_family = AF_UNIX; // Set socket family
    strncpy((*server_address).sun_path, SOCKET_PATH, sizeof((*server_address).sun_path) - 1); // Copy path
    syslog(LOG_NOTICE, "[+]serverSetup Memset worked\n");
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
uint8_t respond(int fd, uint8_t r){
    send(fd,&r,sizeof(r),0);
    return r;
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
            exit(EXIT_FAILURE); // change here to RES_FAIL
            

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




int main(){
    printf("About to daemonsize\n");
    openlog("DAEMON", LOG_PID, LOG_DAEMON);
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