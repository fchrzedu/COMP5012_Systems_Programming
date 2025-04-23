#ifndef LIB_H
#define LIB_H

#include <stdint.h>

#define SOCKET_PATH "/tmp/daemon_unix_socket"
int connectDaemon();
void logOpen();


#endif // LIB_H