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
#define CLOSE 5

/* ----- COCKET CONNECIVITY & DATA LOGGING ----- */
int connectDaemon(); // Connnects each client directly to daemon socket
void logOpen(); // opens logging for each client
uint8_t handleErr(int fd); // return (uint8_t) fail 

/* ----- API HELPER FUNCTIONS  ----- */
bool sendOpCode(int fd, uint8_t opcode); /* send operation code to daemon */
bool sendIDAndLength(int fd, char*ID, int flag); /* sends ID and length */
bool sendSecret(int fd, uint8_t *secret); /* sends secret */
bool sendDataLength(int fd, uint32_t d_len); /* sends data length */
bool sendActualData(int fd, void *d, uint32_t d_len); /* sends data */
uint8_t receiveResponse(int fd); /* receives daemon repsonse and handles */
bool receiveDataLength(int fd, uint32_t *d_len); /* receives data length */
bool receiveActualData(int fd, void *b, uint32_t check_len); /* receives data payload */

/* -- USED BY PARTIALGETBLOCK() -- */
bool sendBeginText(int fd, uint32_t begin_text); /* sends positioning offset */
bool sendLengthText(int fd, uint32_t length_text); /* sends length of text for partialGetBlock */
bool receivePartData(int fd, void *buff, uint32_t len); /* captures snipped data */

/* --- API IPC COMMUNICATION FUNCTIONS ---  */
uint8_t partialGetBlock(char *ID, uint8_t *secret, void **bufferAccess, uint32_t *begin_text, uint32_t length_text); /* partially retrieves data */
uint8_t sendNewBlock(char *ID, uint8_t *secret, uint32_t data_length, void *data); /* sends daemon a new block */
uint8_t getBlock(char *ID, uint8_t *secret, uint32_t buffer_size, void *buffer); /* retrieves data from daemon */
uint8_t overwriteBlock(char *ID, uint8_t *secret, uint32_t data_len, void *data); /* overwrites a block's data */

/* --- CLIENT HELPER FUNCTIONS --- */
void clientMenu();
void readClientSecret(uint8_t *s);
void readClientID(char *id, size_t size);
void readClientData(char *data, size_t size);
void handleClientResponse(uint8_t resp, const char *successMsg, const char *failureMsg);
bool isValidSecret(const char *input);
void readClientSecret(uint8_t *s);

#endif // LIB_H