cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(AirplayStreamer VERSION 0.2.0)

# 设置构建类型
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "")
endif ()
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
        libavcodec
        libavformat
        libavutil
)


# 添加库子目录
add_subdirectory(lib/playfair)
add_subdirectory(lib/llhttp)
add_subdirectory(lib)

# 定义我们的新库
add_library(airplay_streamer src/airplay_streamer.cpp)

# 使用相对路径而不是绝对路径
target_include_directories(airplay_streamer PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib>
        $<INSTALL_INTERFACE:include>
)

# 链接依赖库（原始airplay库中的组件）
target_link_libraries(airplay_streamer PRIVATE airplay playfair llhttp PkgConfig::FFMPEG)

# Test executable
add_executable(airplay_test test/test.cpp)
target_link_libraries(airplay_test PRIVATE airplay_streamer PkgConfig::FFMPEG)

# Install test executable
install(TARGETS airplay_test
        RUNTIME DESTINATION bin)

# 安装规则
install(TARGETS airplay_streamer
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

install(DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")

# 为依赖库添加导出集
install(TARGETS airplay playfair llhttp
        EXPORT AirplayStreamerTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

# 为主库添加导出集
install(TARGETS airplay_streamer
        EXPORT AirplayStreamerTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

# 配置打包
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/AirplayStreamerConfigVersion.cmake"
        VERSION 1.0
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "cmake/AirplayStreamerConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/AirplayStreamerConfig.cmake"
        INSTALL_DESTINATION lib/cmake/AirplayStreamer
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/AirplayStreamerConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/AirplayStreamerConfigVersion.cmake"
        DESTINATION lib/cmake/AirplayStreamer
)

install(EXPORT AirplayStreamerTargets
        FILE AirplayStreamerTargets.cmake
        NAMESPACE AirplayStreamer::
        DESTINATION lib/cmake/AirplayStreamer
)
