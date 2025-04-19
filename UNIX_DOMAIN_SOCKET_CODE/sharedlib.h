#ifndef SHAREDLIB_H
#define SHAREDLIB_H

#include <stdint.h>

#define SOCKET_PATH "/tmp/unixdomainsocket"

/* Op code depending on what the daemon MUST provide */
#define CMD_SEND_BLOCK 1 
#define CMD_GET_BLOCK 2
/* Error return failure codes*/
#define RES_SUCCESS 0 /* res = response from daemon */
#define RES_FAILURE 1
#define RES_ACCESS_DENIED 2
#define RES_NOT_FOUND 3
#define RES_ALREADY_EXISTS 4


int connectDaemon();/* Establishes a daemon connection*/

uint8_t handleErr(int sockfd); /* handles RES_FAILURE */

void logOpen();/* opens logging for sharedlib*/

uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data);
uint8_t getBlock(char *ID, uint8_t *secret, uint32_t buffer_size, void *buffer);

/* two helper functions*/
ssize_t sendAllData(int fd, const void *d, size_t d_len);
ssize_t receiveAllData(int fd, void *buff, size_t length);
#endif // SHAREDLIB_H
