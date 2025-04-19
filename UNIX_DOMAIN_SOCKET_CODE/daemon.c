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

#define CMD_SEND_BLOCK 1
#define CMD_GET_BLOCK  2

#define RES_SUCCESS         0
#define RES_FAILURE         1
#define RES_ACCESS_DENIED   2
#define RES_NOT_FOUND       3
#define RES_ALREADY_EXISTS  4

#define SOCKET_PATH "/tmp/unixdomainsocket"
#define MAX_STORAGE 100  // Max number of blocks to store

/* represents each block of data w/ maximum of 100 blocks stored*/
typedef struct {
    char ID[256];
    uint8_t secret[16];
    uint8_t *data;
    uint32_t data_length;
    int is_used;
} DataBlock;

static DataBlock storage[MAX_STORAGE];




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

void serverSetup(struct sockaddr_un *server_address) {
    memset(server_address, 0, sizeof(struct sockaddr_un)); // Clear struct
    (*server_address).sun_family = AF_UNIX; // Set socket family
    strncpy((*server_address).sun_path, SOCKET_PATH, sizeof((*server_address).sun_path) - 1); // Copy path
    syslog(LOG_NOTICE, "[+] Memset worked");
}

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



void connectionHandling(int server_sock) {
    int client_sock;
    struct sockaddr_un client_address;
    socklen_t client_len;
    uint8_t code; uint8_t resp;

    while (1) {
        client_len = sizeof(client_address);
        client_sock = accept(server_sock, (struct sockaddr *)&client_address, &client_len);

        if (client_sock < 0) {
            syslog(LOG_ERR, "[-] Accept() error\n");
            close(client_sock);
            exit(EXIT_FAILURE);
        }

        syslog(LOG_NOTICE, "[+] Accept() success\n");
    }
        
        
}

void cleanup(int server_sock) {
    close(server_sock);
    unlink(SOCKET_PATH);
    closelog();
    syslog(LOG_NOTICE, "[+] Daemon shutdown");
    exit(EXIT_SUCCESS);
}

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
