#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "lib.h"


int main(){
    char *id = "TestId001";
    uint8_t secret[16] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x10
    };
    char *message = "hello daemon!";
    uint32_t data_length = strlen(message);

    uint8_t result = sendNewBlock(id,secret,data_length,message);
    if (result == 0) {
        printf("[+] Block sent successfully.\n");
    } else {
        printf("[-] Failed to send block. Error code: %d\n", result);
    }

    return 0;
}
