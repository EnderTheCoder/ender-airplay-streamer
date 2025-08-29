#ifndef DNSSD_H
#define DNSSD_H

#ifdef __cplusplus
extern "C" {
#endif

    // Windows 下避免和 Bonjour SDK 冲突
#ifdef _WIN32
    // 如果已经包含了 Bonjour SDK 的 dns_sd.h，就不要重新定义
#ifndef _DNS_SD_H
#if defined(WIN32) && defined(DLL_EXPORT)
# define DNSSD_API __declspec(dllexport)
#else
# define DNSSD_API
#endif
#else
    // 使用 Bonjour SDK 的定义
#define DNSSD_API
#endif
#else
    // Linux/Unix 保持原有定义
#if defined(WIN32) && defined(DLL_EXPORT)
# define DNSSD_API __declspec(dllexport)
#else
# define DNSSD_API
#endif
#endif

#define DNSSD_ERROR_NOERROR       0
#define DNSSD_ERROR_HWADDRLEN     1
#define DNSSD_ERROR_OUTOFMEM      2
#define DNSSD_ERROR_LIBNOTFOUND   3
#define DNSSD_ERROR_PROCNOTFOUND  4

    typedef struct dnssd_s dnssd_t;

    DNSSD_API dnssd_t *dnssd_init(const char *name, int name_len, const char *hw_addr, int hw_addr_len, int *error);

    DNSSD_API int dnssd_register_raop(dnssd_t *dnssd, unsigned short port);
    DNSSD_API int dnssd_register_airplay(dnssd_t *dnssd, unsigned short port);

    DNSSD_API void dnssd_unregister_raop(dnssd_t *dnssd);
    DNSSD_API void dnssd_unregister_airplay(dnssd_t *dnssd);

    DNSSD_API const char *dnssd_get_airplay_txt(dnssd_t *dnssd, int *length);
    DNSSD_API const char *dnssd_get_name(dnssd_t *dnssd, int *length);
    DNSSD_API const char *dnssd_get_hw_addr(dnssd_t *dnssd, int *length);

    DNSSD_API void dnssd_destroy(dnssd_t *dnssd);

#ifdef __cplusplus
}
#endif
#endif
