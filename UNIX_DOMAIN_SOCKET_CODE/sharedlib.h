#ifndef SHAREDLIB_H
#define SHAREDLIB_H

#include <stdint.h>

#define SOCKET_PATH "/tmp/unixdomainsocket"

/* Establishes a daemon connection*/
int connectDaemon();

uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data);

uint8_t getBlock(char *ID, uint8_t *secret, uint32_t buffer_size, void *buffer);

#endif // SHAREDLIB_H
