#ifdef _WIN32
#include "windows_compat.h"
#include <windows.h>
#include <time.h>


int clock_gettime(clockid_t clk_id, struct timespec *tp) {
    (void)clk_id;

    static LARGE_INTEGER frequency;
    static LARGE_INTEGER start;
    static BOOL is_qpc_available = -1;

    if (is_qpc_available == -1) {
        is_qpc_available = QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&start);
    }

    if (is_qpc_available) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);

        LONGLONG elapsed = now.QuadPart - start.QuadPart;
        tp->tv_sec = (long)(elapsed / frequency.QuadPart);
        tp->tv_nsec = (long)(((elapsed % frequency.QuadPart) * 1000000000) / frequency.QuadPart);
    } else {
        // 回退到GetSystemTimeAsFileTime
        FILETIME ft;
        ULARGE_INTEGER ui;

        GetSystemTimeAsFileTime(&ft);
        ui.LowPart = ft.dwLowDateTime;
        ui.HighPart = ft.dwHighDateTime;

        // 转换为Unix时间戳 (从1601-01-01到1970-01-01)
        ui.QuadPart -= 116444736000000000ULL;

        // 转换为秒和纳秒
        tp->tv_sec = (long)(ui.QuadPart / 10000000ULL);
        tp->tv_nsec = (long)((ui.QuadPart % 10000000ULL) * 100);
    }

    return 0;
}

/* Windows下的ioctl实现 */
int ioctl(int fd, int cmd, unsigned long *arg) {
    if (cmd == FIONREAD) {
        return ioctlsocket(fd, FIONREAD, arg);
    }
    return -1; /* 不支持其他命令 */
}

/* Windows下的sleep实现 */
unsigned int sleep(unsigned int seconds) {
    Sleep(seconds * 1000);
    return 0;
}

/* Windows下的usleep实现 */
int usleep(unsigned int usec) {
    // 确保至少睡眠1毫秒
    if (usec < 1000) {
        usec = 1000;
    }
    Sleep(usec / 1000);
    return 0;
}

#endif