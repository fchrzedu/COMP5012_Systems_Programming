/* libshare.h */

#ifndef LIBSHARE_H
#define LIBSHARE_H

#include <sys/types.h>
#include <stddef.h>
#include <errno.h> /* perror */

struct sockaddr_in server_addr;
static int daemonConnection();
void initialise_server_addr(const char* server_ip, int port);
int sendNewBlock(const char *id, const char *secret, const char *data);
int getBlock(const char *id, const char *secret, char *buffer, size_t bufsize);

#endif