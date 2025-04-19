#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sharedlib.h" /* shared lib implementation */

int main() {
    char *test_id = "block1";
    uint8_t secret[16] = {0}; // All-zero test secret
    char *msg = "Hello from client!";
    uint32_t msg_len = strlen(msg) + 1;

    uint8_t res = sendNewBlock(test_id, secret, msg_len, msg);
    printf("Send response = %d\n", res);

    char buffer[128];
    res = getBlock(test_id, secret, sizeof(buffer), buffer);
    if (res == RES_SUCCESS) {
        printf("Received block: %s\n", buffer);
    } else {
        printf("Failed to get block. Error code: %d\n", res);
    }

    return 0;
}

