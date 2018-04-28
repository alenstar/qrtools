#include "utils.h"

int bin2hex(const char *bin, size_t len, char *hex, size_t hex_len)
{
    if ((len * 2) > hex_len) {
        return -1;
    }
    char tb_hex[] = "0123456789ABCDEF";
    int idx = 0;
    int i = 0;
    for (; i < len; i++) {
        hex[idx] = tb_hex[((bin[i] >> 4) & 0x0f)];
        hex[idx + 1] = tb_hex[(bin[i] & 0x0f)];
        idx += 2;
    }
    return len * 2;
}

static inline char h2c(char c)
{
    if (c <= '9') {
        return c - '0';
    } else if (c <= 'Z') {
        return (c - 'A') + 0x0A;
    } else if (c <= 'z') {
        return (c - 'a') + 0x0A;
    }
    return (char)0;
}
int hex2bin(const char *hex, size_t len, char *bin, size_t bin_len)
{
    if (len > (bin_len * 2) || (len % 2) != 0) {
        return -1;
    }
    int j = 0;
    int i = 0;
    int cnt = len / 2;
    for (; j < cnt; j++) {
        bin[j] = (h2c(hex[i]) << 4) | h2c(hex[i + 1]);
        i += 2;
    }
    return cnt;
}
void hexdump(const void *buffer, size_t len, char cols)
{
    size_t i = 0;

    for (i = 0; i < len + ((len % cols) ? (cols - len % cols) : 0); i++) {
        /* print hex data */
        if (i < len) {
            printf("%02X ", ((char*)buffer)[i] & 0xFF);
        }

        if (i % cols == (cols - 1)) {
            printf("\n");
        }
    }
}
