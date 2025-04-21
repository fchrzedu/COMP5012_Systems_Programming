#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sharedlib.h"

int main() {
    char *id = "testID";
    uint8_t secret[16] = "supersecret12345";
    char buffer[1024] = {0};

    uint8_t result = getBlock(id, secret, sizeof(buffer), buffer);
    if (result == 0) {
        printf("[+] Block retrieved: %s\n", buffer);
    } else {
        printf("[-] Failed to retrieve block. Code: %u\n", result);
    }

    return 0;
}
