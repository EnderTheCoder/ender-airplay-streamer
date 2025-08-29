cmake_minimum_required(VERSION 3.27)
include_directories(playfair llhttp)

aux_source_directory(. play_src)
set(DIR_SRCS ${play_src})

add_library(airplay
        STATIC
        ${DIR_SRCS}
)

target_link_libraries(airplay
        pthread
        playfair
        llhttp)
if (WIN32)
    find_package(unofficial-libplist CONFIG REQUIRED)
    target_link_libraries(airplay
            unofficial::libplist::libplist
    )
elseif (UNIX)
    find_library(LIBPLIST NAMES plist plist-2.0)

    target_link_libraries(airplay
            ${LIBPLIST})
endif ()
find_package(OpenSSL 1.1.1 REQUIRED)
target_compile_definitions(airplay PUBLIC OPENSSL_API_COMPAT=0x10101000L)
target_link_libraries(airplay OpenSSL::Crypto)
if (UNIX AND NOT APPLE)

    target_link_libraries(airplay dns_sd)
endif ()

if (WIN32)
    include_directories(${mdns_INCLUDE_DIRS})
    target_link_libraries(airplay ws2_32 iphlpapi)
endif ()
