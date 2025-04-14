#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SOCKET_PATH "/tmp/unixdomainsocket"

int main() {
    int sockfd;
    struct sockaddr_un addr;
    char buffer[256];

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    ssize_t num_read = read(sockfd, buffer, sizeof(buffer)-1);
    if (num_read > 0) {
        buffer[num_read] = '\0'; // Null terminate
        printf("Received: %s\n", buffer);
    } else {
        perror("read");
    }

    close(sockfd);
    return 0;
}
