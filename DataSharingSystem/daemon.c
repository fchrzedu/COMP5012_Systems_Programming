#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/testsocket.sock"

int main() {
    int server_fd, client_fd;
    struct sockaddr_un server_addr;

    // Cleanup old socket if it exists
    unlink(SOCKET_PATH);

    // Create UNIX domain socket
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Clear and set up the socket address
    memset(&server_addr, 0, sizeof(struct sockaddr_un));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    // Correctly calculate bind length
    int bind_len = sizeof(server_addr.sun_family) + strlen(server_addr.sun_path);

    if (bind(server_fd, (struct sockaddr *)&server_addr, bind_len) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        close(server_fd);
        unlink(SOCKET_PATH);
        exit(EXIT_FAILURE);
    }

    printf("[+] Server is listening on %s\n", SOCKET_PATH);

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        const char *msg = "---Hello from the daemon!\n";
        write(client_fd, msg, strlen(msg));
        close(client_fd);
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}
