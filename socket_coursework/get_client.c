#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "lib.h"


int main(){
    char *id = "this id ID franek";
    uint8_t secret[16] = {
        0xFF, 0x02, 0x33, 0x04,
        0x55, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x10
    };
    uint8_t buffer[256] = {0};
    uint8_t result = getBlock(id,secret,sizeof(buffer),buffer);
    if (result == SUCCESS) {
        printf("Data retrieved from daemon: %s \n", buffer);
    } else {
        printf("Failed to retrieve block. Error code: %d\n", result);
    }

    return 0;
}

/*
int main() {
    // Example ID to retrieve the block
    uint32_t id = 12345;

    // The same secret array used to send the data
    uint8_t secret[16] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x10
    };
    // Retrieve data from the daemon
    uint8_t *retrieved_data = NULL;
    size_t retrieved_size = 0;
    int result = getBlock(id, secret, &retrieved_data, &retrieved_size);

    if (result == 0 && retrieved_data != NULL) {
        printf("Data retrieved from the daemon: %s\n", retrieved_data);
        free(retrieved_data);  // Don't forget to free the allocated memory
    } else {
        printf("Failed to retrieve data from the daemon.\n");
    }

    return 0;
}
*/