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

typedef struct {
    char ID[100];         // ID string
    uint8_t secret[16];   // 16-byte secret
    uint32_t data_length; // Length of the data
    void *data;           // Pointer to the actual data
} DataBlock;

DataBlock storage[MAX_STORAGE];

int findBlockID(const char *id) {
    for (int i = 0; i < MAX_STORAGE; i++) {
        if (strcmp(storage[i].ID, id) == 0) {
            return i;
        }
    }
    return -1;
}

int findEmpty() {
    for (int i = 0; i < MAX_STORAGE; i++) {
        if (storage[i].ID[0] == '\0') {
            return i;
        }
    }
    return -1;
}

int receiveBytes(int sockfd, void *buffer, size_t size) {
    ssize_t bytes = recv(sockfd, buffer, size, 0);
    if (bytes != size) {
        return 0;
    }
    return 1;
}

void sendResponse(int sockfd, uint8_t code) {
    send(sockfd, &code, sizeof(code), 0);
}

int compareSecrets(uint8_t *a, uint8_t *b) {
    return memcmp(a, b, 16) == 0;
}

void handleSend(int sockfd) {
    char ID[100];
    uint8_t secret[16];
    uint32_t data_length;
    void *data = NULL;

    if (!receiveBytes(sockfd, ID, sizeof(ID))) {
        sendResponse(sockfd, RES_FAILURE);
        exit(EXIT_FAILURE);
    }

    if (!receiveBytes(sockfd, secret, 16)) {
        sendResponse(sockfd, RES_FAILURE);
        exit(EXIT_FAILURE);
    }

    if (!receiveBytes(sockfd, &data_length, sizeof(data_length))) {
        sendResponse(sockfd, RES_FAILURE);
        exit(EXIT_FAILURE);
    }

    data = malloc(data_length);
    if (!data || !receiveBytes(sockfd, data, data_length)) {
        sendResponse(sockfd, RES_FAILURE);
        free(data);
        exit(EXIT_FAILURE);
    }

    int index = findBlockID(ID);
    if (index != -1) {
        sendResponse(sockfd, RES_ALREADY_EXISTS);
        free(data);
        exit(EXIT_SUCCESS);
    }

    index = findEmpty();
    if (index == -1) {
        sendResponse(sockfd, RES_FAILURE);
        free(data);
        exit(EXIT_SUCCESS);
    }

    strncpy(storage[index].ID, ID, sizeof(storage[index].ID));
    memcpy(storage[index].secret, secret, 16);
    storage[index].data_length = data_length;
    storage[index].data = data;

    sendResponse(sockfd, RES_SUCCESS);
    exit(EXIT_SUCCESS);
}

void handleGet(int sockfd) {
    char ID[100];
    uint8_t secret[16];
    uint32_t buffer_size;

    if (!receiveBytes(sockfd, ID, sizeof(ID)) ||
        !receiveBytes(sockfd, secret, 16) ||
        !receiveBytes(sockfd, &buffer_size, sizeof(buffer_size))) {
        sendResponse(sockfd, RES_FAILURE);
        exit(EXIT_FAILURE);
    }

    int index = findBlockID(ID);
    if (index == -1) {
        sendResponse(sockfd, RES_NOT_FOUND);
        exit(EXIT_SUCCESS);
    }

    if (!compareSecrets(secret, storage[index].secret)) {
        sendResponse(sockfd, RES_ACCESS_DENIED);
        exit(EXIT_SUCCESS);
    }

    if (buffer_size < storage[index].data_length) {
        sendResponse(sockfd, RES_FAILURE);
        exit(EXIT_SUCCESS);
    }

    sendResponse(sockfd, RES_SUCCESS);
    send(sockfd, storage[index].data, storage[index].data_length, 0);
    exit(EXIT_SUCCESS);
}

void handleData(int sockfd) {
    uint8_t cmd;
    if (!receiveBytes(sockfd, &cmd, sizeof(cmd))) {
        syslog(LOG_ERR, "[-] Failed to receive command");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (cmd == CMD_SEND_BLOCK) {
        handleSend(sockfd);
    } else if (cmd == CMD_GET_BLOCK) {
        handleGet(sockfd);
    } else {
        syslog(LOG_ERR, "[-] Unknown command");
        sendResponse(sockfd, RES_FAILURE);
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    close(sockfd);
}
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

    while (1) {
        client_len = sizeof(client_address);
        client_sock = accept(server_sock, (struct sockaddr *)&client_address, &client_len);

        if (client_sock < 0) {
            syslog(LOG_ERR, "[-] Accept() error\n");
            close(client_sock);
            exit(EXIT_FAILURE);
        }

        syslog(LOG_NOTICE, "[+] Accept() success\n");
        handleData(client_sock);
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
