#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "lib.h"


int main(){
    char *id = "test id 1008";
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

