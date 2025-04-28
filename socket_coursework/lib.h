#ifndef LIB_H
#define LIB_H

#include <stdint.h>
#include <stdbool.h>

#define SOCKET_PATH "/tmp/domainsocket"

/* ----- LIBRARY SEND OP CODES ----- */
#define SEND 0
#define GET 1
#define PARTIAL_GET 2
#define UPDATE 3

/* ----- DAEMON RESPONSE OP CODES ----- */
#define SUCCESS 0
#define FAIL 1
#define ACCESS_DENIED 2
#define NOT_FOUND 3
#define ALREADY_EXISTS 4

int connectDaemon(); // Connnects each client directly to daemon socket
void logOpen(); // opens logging for each client
uint8_t handleErr(int fd); // return (uint8_t) fail 

/* ----- helper functions for sendNewBlock() & getBlock() ----- */
bool sendOpCode(int fd, uint8_t opcode);
bool sendIDLength(int fd, char*ID);
bool sendOpCode(int fd, uint8_t opcode);
bool sendIDAndLength(int fd, char*ID, int flag);
bool sendSecret(int fd, uint8_t *secret);
bool sendDataLength(int fd, uint32_t d_len);
bool sendActualData(int fd, void *d, uint32_t d_len);
uint8_t receiveResponse(int fd);
bool receiveDataLength(int fd, uint32_t *d_len);
bool receiveActualData(int fd, void *b, uint32_t check_len);
/* -- USED BY PARTIALGETBLOCK() -- */
bool sendBeginText(int fd, uint32_t begin_text);
bool sendLengthText(int fd, uint32_t length_text);
bool receivePartData(int fd, void *buff, uint32_t len);

/* --- IPC COMMUNICATION FUNCTIONS ---  */
uint8_t partialGetBlock(char *ID, uint8_t *secret, void **bufferAccess, uint32_t *begin_text, uint32_t length_text);
uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data); // Sends a new block
uint8_t getBlock(char *ID, uint8_t *secret, uint32_t buffer_size, void *buffer); // Retreives block from daemon - prints to client
uint8_t overwriteBlock(char *ID, uint8_t *secret, uint32_t data_len, void *data);

#endif // LIB_H