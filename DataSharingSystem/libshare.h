// libshare.h

#ifndef LIBSHARE_H
#define LIBSHARE_H

#include <sys/types.h>
#include <stddef.h>

int sendNewBlock(const char *id, const char *secret, const char *data);
int getBlock(const char *id, const char *secret, char *buffer, size_t bufsize);

#endif