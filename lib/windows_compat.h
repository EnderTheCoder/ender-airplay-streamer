#ifndef WINDOWS_COMPAT_H
#define WINDOWS_COMPAT_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

// 定义FIONREAD
#ifndef FIONREAD
#define FIONREAD 0x4004667F
#endif

// // 检查是否已经定义了timespec结构
// #ifndef TIMESPEC_DEFINED
// #define TIMESPEC_DEFINED
// // 只有在新版本Windows SDK中没有定义timespec时才定义
// #if !defined(_TIMESPEC) && !defined(__TIMESPEC_IMPL)
// struct timespec {
//     time_t tv_sec;  // 秒
//     long   tv_nsec; // 纳秒
// };
// #endif
// #endif

// Windows下的ioctl实现
int ioctl(int fd, int cmd, unsigned long *arg);

// Windows下的clock_gettime实现
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif
typedef int clockid_t;
int clock_gettime(clockid_t clk_id, struct timespec *tp);

// Windows下的sleep实现
unsigned int sleep(unsigned int seconds);

// Windows下的usleep实现
int usleep(unsigned int usec);

// 关闭套接字的宏
#define closesocket(s) closesocket(s)

#else
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>

#define closesocket(s) close(s)

#endif

#endif /* WINDOWS_COMPAT_H */