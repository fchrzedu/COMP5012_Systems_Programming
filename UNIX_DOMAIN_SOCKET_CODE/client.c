#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sharedlib.h"  // make sure this matches your API

int main() {
    const char *block_id = "testblocak003";
    uint8_t secret[16] = {
        0x11, 0x20, 0x30, 0x40,
        0x50, 0x60, 0x70, 0x80,
        0x90, 0xAE, 0xB0, 0xC0,
        0xD0, 0xE0, 0xF0, 0x01
    };
    const char *payload = "hi";
    uint32_t payload_len = strlen(payload);

    printf("Sending block ID: %s\n", block_id);
    printf("Secret: ");
    for (int i = 0; i < sizeof(secret); i++) {
        printf("%02X ", secret[i]);
    }
    printf("\n");
    printf("Payload Length: %u\n", payload_len);
    printf("Payload: %s\n", payload);
    
    uint8_t status = sendNewBlock(block_id,secret,payload_len,(void*)payload);
    if (status == 0) {
        printf("[+] Block successfully sent to daemon.\n");
    } else if (status == 4) {
        printf("[!] Block already exists.\n");
    } else {
        printf("[-] Failed to send block. Error code: %d\n", status);
    }

    return 0;
}
