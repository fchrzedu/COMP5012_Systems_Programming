#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sharedlib.h" /* shared lib implementation */

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
