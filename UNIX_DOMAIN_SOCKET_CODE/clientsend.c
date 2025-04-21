#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sharedlib.h"  
int main() {
    char *id = "testID";
    uint8_t secret[16] = "supersecret12345";
    char *message = "Hello from client_send!";
    uint32_t len = strlen(message) + 1; // include null terminator

    uint8_t result = sendNewBlock(id, secret, len, message);
    if (result == 0) {
        printf("[+] Block sent successfully.\n");
    } else {
        printf("[-] Failed to send block. Code: %u\n", result);
    }

    return 0;

    return 0;
}
