#ifndef LIB_H
#define LIB_H

#include <stdint.h>
#include <stdbool.h>

#define SOCKET_PATH "/tmp/domainsocket"

/* ----- SHARED LIB SEND OP CODES ----- */
#define SEND 0
#define GET 1

/* ----- DAEMON RESPONSE OP CODES ----- */
#define SUCCESS 0
#define FAIL 1
#define ACCESS_DENIED 2
#define NOT_FOUND 3
#define ALREADY_EXISTS 4

/* ----- functions & methods ----- */
int connectDaemon(); // Client socket side connection
void logOpen(); // Opens logging for shared library
uint8_t handleErr(int fd); // Handles response FAIL

/* ----- helper functions for sendNewBlock() & getBlock() ----- */
bool sendOpCode(int fd, uint8_t opcode);
bool sendIDLength(int fd, char*ID);


uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data); // Sends a new block
uint8_t getBlock(char *ID, uint8_t *secret, uint32_t buffer_size, void *buffer); // Retreives block from daemon - prints to client


#endif // LIB_H