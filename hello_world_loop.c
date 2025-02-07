#include <stdio.h>
#include <stdlib.h> // For atoi()

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    int count = atoi(argv[1]); // Convert string to integer
    if (count <= 0) {
        printf("Please enter a valid positive number.\n");
        return 1;
    }

    for (int i = 0; i < count; i++) {
        puts("Hello Minix.");
    }

    return 0;
}