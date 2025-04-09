#ifndef LIBSHARE_H
#define LIBSHARE_H

#define PORT 9002
#define IP "127.0.0.1" /* Retrieved from telnet localhost 9002 */
#define BLOCK_SIZE 1024 /* Arbitary length to provide max length on secret*/
#define SECRET_SIZE 64 /* 16 byte */

/* Const to not change parameters */

/* Used to send data to the daemon*/
uint8_t sendNewBlock(const char *ID, const uint8_t *secret, const uint32_t data_length, const char *data);


/* Retrieves data from daemon */
uint8_t getBlock(const char *ID, const uint8_t *secret, const uint32_t buffer_size, const char *buffer);

/* Creates  a client socket connection to the dameon */
int daemonConnect(); 

void log(const char *message); /* Used to log any errors in client socket */

//void initialiseLog();

#endif