#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lib.h"


int main() {
    bool has_exit = false;
    while (!has_exit) {
        printf("\033[H\033[J"); /* Clears MINIX and most terminals*/
        clientMenu();
        int user_choice = 0;
        scanf("%d", &user_choice);
        getchar(); // Clears buffer for \0

        char id[256] = {0};
        uint8_t secret[16] = {0};
        char data[512] = {0};
        uint32_t data_length = 0;

        switch (user_choice) {
            case 1: // sendNewBlock()
                readClientID(id, sizeof(id));
                readClientSecret(secret);
                readClientData(data, sizeof(data));
                data_length = strlen(data);
                uint8_t resp = sendNewBlock(id, secret, data_length, data);
                handleClientResponse(resp, "Block added successfully.", "Failed to add block.");
                sleep(2);
                break;

            case 2: // overwriteBlock()
                readClientID(id, sizeof(id));
                readClientSecret(secret);
                readClientData(data, sizeof(data));
                data_length = strlen(data);
                resp = overwriteBlock(id, secret, data_length, data);
                handleClientResponse(resp, "Block updated successfully.", "Failed to update block.");
                sleep(2);
                break;

            case 3: // partialGetBlock()
                readClientID(id, sizeof(id));
                readClientSecret(secret);

                uint32_t begin_text_offset, length_text = 0;
                printf("Enter integer offset to start at> ");
                scanf("%u", &begin_text_offset);
                printf("Enter length of data to retrieve> ");
                scanf("%u", &length_text);
                getchar();

                char *partial_payload = NULL;
                resp = partialGetBlock(id, secret, (void **)&partial_payload, &begin_text_offset, length_text);

                if (resp == SUCCESS) {
                    printf("[+] Partial data received: %s\n", partial_payload);
                    free(partial_payload);
                } else {
                    printf("[-] Failed to receive partial data for ID: %s\n", id);
                }
                sleep(2);
                break;

            case 4: // getBlock()
                readClientID(id, sizeof(id));
                readClientSecret(secret);

                char idbuff[256] = {0};
                resp = getBlock(id, secret, sizeof(idbuff), idbuff);
                if (resp == SUCCESS) {
                    printf("[+] Data retrieved successfully: %s\n", idbuff);
                } else {
                    printf("[-] Failed to retrieve data.\n");
                }
                sleep(2);
                break;

            case 5: // Disconnect
                printf("[!] Disconnecting client\n");
                sleep(2);
                has_exit = true;
                break;

            default:
                printf("Please enter a valid choice, it must be a number from the menu\n");
                sleep(2);
                break;
        }
    }
}