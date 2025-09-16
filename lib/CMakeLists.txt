cmake_minimum_required(VERSION 3.27)
include_directories(playfair llhttp)

aux_source_directory(. play_src)
set(DIR_SRCS ${play_src})

add_library(airplay
        STATIC
        ${DIR_SRCS}
)

target_link_libraries(airplay
        playfair
        llhttp)

if (WIN32)

    target_compile_definitions(airplay PRIVATE WIN32_LEAN_AND_MEAN _WINSOCK_DEPRECATED_NO_WARNINGS)

    find_package(unofficial-libplist CONFIG REQUIRED)
    target_link_libraries(airplay
            unofficial::libplist::libplist
    )
    find_package(PThreads4W REQUIRED)

    # 🔧 添加 Bonjour SDK 支持
    # 方法1: 查找已安装的 Bonjour SDK
    find_path(BONJOUR_INCLUDE_DIR dns_sd.h
            PATHS
            "C:/Program Files/Bonjour SDK/Include"
            "C:/Program Files (x86)/Bonjour SDK/Include"
            ENV INCLUDE
    )

    find_library(BONJOUR_LIBRARY
            NAMES dnssd
            PATHS
            "C:/Program Files/Bonjour SDK/Lib/x64"
            "C:/Program Files (x86)/Bonjour SDK/Lib/x64"
            ENV LIB
    )

    if (BONJOUR_INCLUDE_DIR AND BONJOUR_LIBRARY)
        target_include_directories(airplay PRIVATE ${BONJOUR_INCLUDE_DIR})
        target_link_libraries(airplay ${BONJOUR_LIBRARY})
        message(STATUS "Found Bonjour SDK: ${BONJOUR_INCLUDE_DIR}")
    else ()
        # 方法2: 如果没有安装 SDK，使用动态加载方式
        # 你的代码已经支持动态加载，所以这里可以继续编译
        message(WARNING "Bonjour SDK not found, will use dynamic loading")
        # 添加必要的 Windows API
        target_link_libraries(airplay ws2_32 iphlpapi)
    endif ()

elseif (UNIX)
    find_library(LIBPLIST NAMES plist plist-2.0)
    target_link_libraries(airplay ${LIBPLIST} pthread)
endif ()

find_package(OpenSSL 1.1.1 REQUIRED)
target_compile_definitions(airplay PUBLIC OPENSSL_API_COMPAT=0x10101000L)
target_link_libraries(airplay OpenSSL::Crypto)

if (UNIX AND NOT APPLE)
    target_link_libraries(airplay dns_sd)
endif ()

if (WIN32)
    target_link_libraries(airplay ws2_32 iphlpapi PThreads4W::PThreads4W)
endif ()
