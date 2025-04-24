#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "lib.h"

int main() {
    char *testID = "block123";
    uint8_t secret[16] = {
        0xde, 0xad, 0xbe, 0xef,
        0xca, 0xfe, 0xba, 0xbe,
        0x00, 0x11, 0x22, 0x33,
        0x44, 0x55, 0x66, 0x77
    };

    char testData[] = "Hello, daemon. This is a test block!";
    uint32_t dataLength = strlen(testData) + 1;  // include null terminator

    uint8_t result = sendNewBlock(testID, secret, dataLength, testData);

    if (result == SUCCESS) {
        printf("[+] Block sent successfully!\n");
    } else {
        printf("[-] Failed to send block to daemon.\n");
    }

    return 0;
}
