#ifndef LIB_H
#define LIB_H

#include <stdint.h>

#define SOCKET_PATH "/tmp/daemon_unix_socket"

/* ----- SHARED LIB SEND OP CODES ----- */
#define SEND 0
#define GET 1

/* ----- DAEMON RESPONSE OP CODES ----- */
#define SUCCESS 0
#define FAIL 1

/* ----- functions & methods ----- */
int connectDaemon();
void logOpen();

uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data);
uint8_t getBlock(char *ID, uint8_t *secret, uint32_t buffer_size, void *buffer);


#endif // LIB_H