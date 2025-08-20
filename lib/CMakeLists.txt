cmake_minimum_required(VERSION 3.27)
include_directories(playfair llhttp)

aux_source_directory(. play_src)
set(DIR_SRCS ${play_src})

add_library(airplay
        STATIC
        ${DIR_SRCS}
)
if (WIN32)
    find_package(unofficial-libplist CONFIG REQUIRED)
    target_link_libraries(airplay
            pthread
            playfair
            llhttp
            PRIVATE unofficial::libplist::libplist
    )
elseif (UNIX)
    find_library(LIBPLIST NAMES plist plist-2.0)

    target_link_libraries(airplay
            pthread
            playfair
            llhttp
            ${LIBPLIST})
endif ()
if (UNIX AND NOT APPLE)
    find_package(OpenSSL 1.1.1 REQUIRED)
    target_compile_definitions(airplay PUBLIC OPENSSL_API_COMPAT=0x10101000L)
    target_link_libraries(airplay OpenSSL::Crypto)
    target_link_libraries(airplay dns_sd)
else ()
    include_directories(/usr/local/opt/openssl@1.1/include/)
    target_link_libraries(airplay /usr/local/opt/openssl@1.1/lib/libcrypto.a)
endif ()
        
