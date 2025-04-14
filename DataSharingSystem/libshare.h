#ifndef libshare_h
#define libshare_h


int connect_to_daemon();
uint8_t sendNewBlock(const char *ID, const uint8_t *secret, const uint32_t data_length, const char *data);
uint8_t getBlock(const char *ID, const uint8_t *secret, const uint32_t buffer_size, const char *buffer);



#endif