#ifndef BYTEUTILS_H
#define BYTEUTILS_H

#include <stdint.h>
// 定义 NTP 时间转换常量
#define SECONDS_FROM_1900_TO_1970 2208988800ULL
#ifdef __cplusplus
extern "C" {
#endif

    uint16_t byteutils_get_short(unsigned char* b, int offset);
    uint32_t byteutils_get_int(unsigned char* b, int offset);
    uint64_t byteutils_get_long(unsigned char* b, int offset);

    uint16_t byteutils_get_short_be(unsigned char* b, int offset);
    uint32_t byteutils_get_int_be(unsigned char* b, int offset);
    uint64_t byteutils_get_long_be(unsigned char* b, int offset);

    float byteutils_get_float(unsigned char* b, int offset);
    void byteutils_put_int(unsigned char* b, int offset, uint32_t value);

    uint64_t byteutils_get_ntp_timestamp(unsigned char *b, int offset);
    void byteutils_put_ntp_timestamp(unsigned char *b, int offset, uint64_t us_since_1970);

#ifdef __cplusplus
}
#endif

#endif // BYTEUTILS_H
