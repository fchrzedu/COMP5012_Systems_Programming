#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "lib.h"  

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include "lib.h"

#define OPCODE_TEST 0x01  // Arbitrary opcode for testing

int main() {
    int sockfd = connectDaemon();
    if (sockfd < 0) {
        fprintf(stderr, "[-] Failed to connect to daemon.\n");
        return EXIT_FAILURE;
    }

    uint8_t opcode = OPCODE_TEST;
    ssize_t sent = send(sockfd, &opcode, sizeof(opcode), 0);
    if (sent != sizeof(opcode)) {
        syslog(LOG_ERR, "[-] Failed to send opcode to daemon\n");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("[+] Sent opcode %d to daemon\n", opcode);
    close(sockfd);
    return EXIT_SUCCESS;
}
