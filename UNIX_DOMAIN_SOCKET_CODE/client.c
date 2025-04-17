#include <stdio.h>
#include "sharedlib.h"

int main() {
    int sockfd = connectDaemon();
    if (sockfd < 0) {
        printf("Connection to daemon failed.\n");
        return 1;
    }

    printf("Connected to daemon! socket fd: %d\n", sockfd);
    close(sockfd);
    return 0;
}
