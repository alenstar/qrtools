#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


int bin2hex(const char *bin, size_t len, char *hex, size_t hex_len);
int hex2bin(const char *hex, size_t len, char *bin, size_t bin_len);
void hexdump(const void *buffer, size_t len, char cols);

#ifdef __cplusplus
}
#endif

#endif // UTILS_H
