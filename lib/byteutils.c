/*
 * Copyright (c) 2019 dsafa22, All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#include "byteutils.h"

#include <time.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <netinet/in.h>
#endif



// 跨平台的 64 位网络字节序转换
#ifndef htonll
    #ifdef _WIN32
        #ifdef _MSC_VER
            #include <stdlib.h>
            #define htonll(x) _byteswap_uint64(x)
            #define ntohll(x) _byteswap_uint64(x)
        #else
            // MinGW 或其他 Windows 编译器
            #define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl((x) >> 32))
            #define ntohll(x) ((((uint64_t)ntohl(x)) << 32) + ntohl((x) >> 32))
        #endif
    #else
        // Unix-like systems
        #include <endian.h>
        #if __BYTE_ORDER == __BIG_ENDIAN
            #define htonll(x) (x)
            #define ntohll(x) (x)
        #else
            #define htonll(x) htobe64(x)
            #define ntohll(x) be64toh(x)
        #endif
    #endif
#endif

// 安全的字节序转换函数 - 小端格式
static inline uint16_t load_le16(const unsigned char* buf) {
    return (uint16_t)(buf[0] | (buf[1] << 8));
}

static inline uint32_t load_le32(const unsigned char* buf) {
    return (uint32_t)(buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
}

static inline uint64_t load_le64(const unsigned char* buf) {
    return ((uint64_t)buf[0] | ((uint64_t)buf[1] << 8) |
            ((uint64_t)buf[2] << 16) | ((uint64_t)buf[3] << 24) |
            ((uint64_t)buf[4] << 32) | ((uint64_t)buf[5] << 40) |
            ((uint64_t)buf[6] << 48) | ((uint64_t)buf[7] << 56));
}

static inline float load_le_float(const unsigned char* buf) {
    union {
        uint32_t i;
        float f;
    } u;
    u.i = load_le32(buf);
    return u.f;
}

// 安全的字节序转换函数 - 大端格式
static inline uint16_t load_be16(const unsigned char* buf) {
    return (uint16_t)((buf[0] << 8) | buf[1]);
}

static inline uint32_t load_be32(const unsigned char* buf) {
    return (uint32_t)((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]);
}

static inline uint64_t load_be64(const unsigned char* buf) {
    return ((uint64_t)buf[0] << 56) | ((uint64_t)buf[1] << 48) |
           ((uint64_t)buf[2] << 40) | ((uint64_t)buf[3] << 32) |
           ((uint64_t)buf[4] << 24) | ((uint64_t)buf[5] << 16) |
           ((uint64_t)buf[6] << 8) | (uint64_t)buf[7];
}

// 安全的数据写入函数 - 小端格式
static inline void store_le32(unsigned char* buf, uint32_t value) {
    buf[0] = (unsigned char)(value & 0xFF);
    buf[1] = (unsigned char)((value >> 8) & 0xFF);
    buf[2] = (unsigned char)((value >> 16) & 0xFF);
    buf[3] = (unsigned char)((value >> 24) & 0xFF);
}

/**
 * Reads a little endian unsigned 16 bit integer from the buffer at position offset
 */
uint16_t byteutils_get_short(unsigned char* b, int offset) {
    return load_le16(b + offset);
}

/**
 * Reads a little endian unsigned 32 bit integer from the buffer at position offset
 */
uint32_t byteutils_get_int(unsigned char* b, int offset) {
    return load_le32(b + offset);
}

/**
 * Reads a little endian unsigned 64 bit integer from the buffer at position offset
 */
uint64_t byteutils_get_long(unsigned char* b, int offset) {
    return load_le64(b + offset);
}

/**
 * Reads a big endian unsigned 16 bit integer from the buffer at position offset
 */
uint16_t byteutils_get_short_be(unsigned char* b, int offset) {
    return load_be16(b + offset);
}

/**
 * Reads a big endian unsigned 32 bit integer from the buffer at position offset
 */
uint32_t byteutils_get_int_be(unsigned char* b, int offset) {
    return load_be32(b + offset);
}

/**
 * Reads a big endian unsigned 64 bit integer from the buffer at position offset
 */
uint64_t byteutils_get_long_be(unsigned char* b, int offset) {
    return load_be64(b + offset);
}

/**
 * Reads a float from the buffer at position offset
 */
float byteutils_get_float(unsigned char* b, int offset) {
    return load_le_float(b + offset);
}

/**
 * Writes a little endian unsigned 32 bit integer to the buffer at position offset
 */
void byteutils_put_int(unsigned char* b, int offset, uint32_t value) {
    store_le32(b + offset, value);
}

/**
 * Reads an ntp timestamp and returns it as micro seconds since the Unix epoch
 */
uint64_t byteutils_get_ntp_timestamp(unsigned char *b, int offset) {
    uint64_t seconds = load_be32(b + offset) - SECONDS_FROM_1900_TO_1970;
    uint64_t fraction = load_be32(b + offset + 4);
    return (seconds * 1000000ULL) + ((fraction * 1000000ULL) >> 32);
}

/**
 * Writes a time given as micro seconds since the Unix time epoch as an ntp timestamp
 * into the buffer at position offset
 */
void byteutils_put_ntp_timestamp(unsigned char *b, int offset, uint64_t us_since_1970) {
    uint64_t seconds = us_since_1970 / 1000000ULL;
    uint64_t microseconds = us_since_1970 % 1000000ULL;
    seconds += SECONDS_FROM_1900_TO_1970;
    uint64_t fraction = (microseconds << 32) / 1000000ULL;

    // Write in big endian!
    store_le32(b + offset, htonl((uint32_t)seconds));
    store_le32(b + offset + 4, htonl((uint32_t)fraction));
}
