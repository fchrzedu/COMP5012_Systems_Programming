#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "lib.h"


int main(){
    char *id = "TEST ID 1289312398";
    uint8_t secret[16] = {
        0xAA, 0x77, 0x33, 0x04,
        0x55, 0x06, 0x07, 0x08,
        0x09, 0x6B, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x10
    };

    uint8_t *buffer = {0};  // buffer must be a pointer
    uint32_t offset = 5;
    uint32_t length = 11;
    uint8_t result = partialGetBlock(id, secret, (void**)&buffer, &offset, length);
    if (result == SUCCESS) {
       
        printf("Data retrieved from daemon: %s\n", buffer);
        free(buffer);
    } else {
        printf("Failed to retrieve block. Error code: %d\n", result);
    }/*
    uint8_t buffer[256] = {0};
    uint8_t result = getBlock(id,secret,sizeof(buffer),buffer);
    if (result == SUCCESS) {
        printf("Data retrieved from daemon: %5s \n", buffer);
    } else {
        printf("Failed to retrieve block. Error code: %d\n", result);
    }*/


    return 0;
}
/*
char *id = "HELLO HOW ARE YOU GUYS!";
    uint8_t secret[16] = {
        0xFF, 0x02, 0x33, 0x04,
        0x55, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x10
    };

    

    return 0;
*/
