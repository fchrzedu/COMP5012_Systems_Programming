#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "lib.h"


int main(){
    char *id = "TEST ID 1289312398";
    uint8_t secret[16] = {
        0xAA, 0x77, 0x33, 0x04,
        0x55, 0x06, 0x07, 0x08,
        0x09, 0x6B, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x10
    };
    char *message = "8348232 R6B br^*r^b*( 6RB)";

    char *new_data = "this block has been overwritten";
    uint32_t data_length = strlen(new_data);

    uint8_t result = overwriteBlock(id, secret, data_length, new_data);
    if (result == SUCCESS) {
        printf("[+] Successfully overwrote block with ID '%s'\n", id);
    } else if (result == FAIL) {
        printf("[-] Failed to overwrite block with ID '%s'\n", id);
    } else {
        printf("[!] Received unexpected response code: %d\n", result);
    }

    return 0;
}
