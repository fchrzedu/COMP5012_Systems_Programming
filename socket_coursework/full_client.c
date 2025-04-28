#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "lib.h"

// Display the menu options
void displayMenu() {
    printf("\n--- Client Menu ---\n");
    printf("1. Send new block\n");
    printf("2. Update an existing block\n");
    printf("3. Retrieve partial information about a block\n");
    printf("4. Retrieve a whole block's data\n");
    printf("5. Exit\n");
    printf("Enter your choice: ");
}

// Helper function to read a secret from the user
void readSecret(uint8_t *secret) {
    printf("Enter secret (16 bytes as hexadecimal, space-separated): ");
    for (int i = 0; i < 16; i++) {
        scanf("%hhx", &secret[i]);
    }
    getchar(); // Consume the newline character
}

int main() {
    while (1) {
        int choice;
        displayMenu();
        scanf("%d", &choice);
        getchar(); // Consume the newline character

        char id[256] = {0};
        uint8_t secret[16] = {0};
        char data[1024] = {0};
        uint32_t data_length = 0;

        switch (choice) {
            case 1: // Send new block
                printf("Enter ID: ");
                fgets(id, sizeof(id), stdin);
                id[strcspn(id, "\n")] = '\0';

                readSecret(secret);

                printf("Enter data: ");
                fgets(data, sizeof(data), stdin);
                data[strcspn(data, "\n")] = '\0';
                data_length = strlen(data);

                if (sendNewBlock(id, secret, data_length, data) == SUCCESS) {
                    printf("[+] Block added successfully.\n");
                } else {
                    printf("[-] Failed to add block.\n");
                }
                break;

            case 2: // Update an existing block
                printf("Enter ID: ");
                fgets(id, sizeof(id), stdin);
                id[strcspn(id, "\n")] = '\0';

                readSecret(secret);

                printf("Enter new data: ");
                fgets(data, sizeof(data), stdin);
                data[strcspn(data, "\n")] = '\0';
                data_length = strlen(data);

                if (overwriteBlock(id, secret, data_length, data) == SUCCESS) {
                    printf("[+] Block updated successfully.\n");
                } else {
                    printf("[-] Failed to update block.\n");
                }
                break;

            case 3: // Retrieve partial information about a block
                printf("Enter ID: ");
                fgets(id, sizeof(id), stdin);
                id[strcspn(id, "\n")] = '\0';

                readSecret(secret);

                uint32_t begin_text, length_text;
                printf("Enter starting offset: ");
                scanf("%u", &begin_text);
                printf("Enter length of data to retrieve: ");
                scanf("%u", &length_text);
                getchar(); // Consume the newline character

                char *partial_data = NULL;
                if (partialGetBlock(id, secret, (void **)&partial_data, &begin_text, length_text) == SUCCESS) {
                    printf("[+] Partial data retrieved: %s\n", partial_data);
                    free(partial_data);
                } else {
                    printf("[-] Failed to retrieve partial data.\n");
                }
                break;

            case 4: // Retrieve a whole block's data
                printf("Enter ID: ");
                fgets(id, sizeof(id), stdin);
                id[strcspn(id, "\n")] = '\0';

                readSecret(secret);

                char buffer[1024];
                if (getBlock(id, secret, sizeof(buffer), buffer) == SUCCESS) {
                    printf("[+] Full data retrieved: %s\n", buffer);
                } else {
                    printf("[-] Failed to retrieve full data.\n");
                }
                break;

            case 5: // Exit
                printf("[+] Exiting client.\n");
                return 0;

            default:
                printf("[-] Invalid choice. Please try again.\n");
                break;
        }
    }
}