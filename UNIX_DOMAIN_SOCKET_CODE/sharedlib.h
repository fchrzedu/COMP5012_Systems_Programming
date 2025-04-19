#ifndef SHAREDLIB_H
#define SHAREDLIB_H

#include <stdint.h>

#define SOCKET_PATH "/tmp/unixdomainsocket"

/
/* OP & response codes to communicate with daemon to client */
#define SEND_BLOCK 0
#define GET_BLOCK 1

#define SUCCESS_RES 0
#define FAIL_RES 1
#define ACCESS_DENIED_RES 2
#define NOT_FOUND_RES 3
#define ALREADY_EXISTS_RED 4


int connectDaemon();/* Establishes a daemon connection*/

uint8_t handleErr(int sockfd); /* handles RES_FAILURE */

void logOpen();/* opens logging for sharedlib*/

uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data);
uint8_t getBlock(char *ID, uint8_t *secret, uint32_t buffer_size, void *buffer);

/* two helper functions - used in getBlock() & sendNewBlock() */
ssize_t sendAllData(int fd, const void *d, size_t d_len);
ssize_t receiveAllData(int fd, void *buff, size_t length);

#endif // SHAREDLIB_H
