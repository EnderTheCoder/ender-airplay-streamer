# Ender Airplay Streamer

A C++ library for receiving and processing AirPlay video streams from iOS devices. This library provides a simple interface to capture H.264 video frames streamed from iPhones and iPads, making it easy to integrate AirPlay functionality into your applications.

## Features

- **Video Streaming**: Receive H.264 video streams from iOS devices via AirPlay
- **Frame Processing**: Access decoded video frames for further processing
- **Cross-platform**: Built with CMake for easy integration
- **Callback-based**: Asynchronous frame delivery through customizable callbacks
- **Logging**: Configurable logging system for debugging and monitoring

## Prerequisites

### System Dependencies

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake ninja-build pkg-config
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libavutil-dev
sudo apt-get install libssl-dev

# macOS (with Homebrew)
brew install cmake ninja ffmpeg openssl
```

### Required Libraries

This project depends on the RPiPlay library (included as submodules):
- `lib/playfair` - AirPlay authentication and protocol handling
- `lib/llhttp` - HTTP parsing library
- `lib` - Main RPiPlay AirPlay implementation

## Building

```bash
# Clone the repository with submodules
git clone --recursive https://github.com/EnderTheCoder/ender-airplay-streamer

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project
make -j$(nproc)

# Or using Ninja (faster builds)
cmake .. -DCMAKE_BUILD_TYPE=Release -G Ninja
ninja
```

## Usage

### Basic Example

```cpp
#include <airplay_streamer.hpp>
#include <iostream>
#include <memory>

int main() {
    using namespace airplay_streamer;
    
    // Configure the streamer
    Config config;
    config.server_name = "MyAirPlayServer";
    config.hw_address = {0x48, 0x5D, 0x60, 0x7C, 0xEE, 0x22}; // Unique MAC address
    
    // Set up video frame callback
    config.on_video_data = [](std::shared_ptr<AVFrame> frame, int64_t timestamp) {
        std::cout << "Received video frame: " 
                  << frame->width << "x" << frame->height 
                  << " at timestamp " << timestamp << std::endl;
        
        // Process the video frame here
        // Access frame data through frame->data[0], frame->linesize[0], etc.
    };
    
    // Optional: Set up logging
    config.log_callback = [](LogLevel level, const char* message) {
        std::string level_str;
        switch(level) {
            case LogLevel::Debug:   level_str = "DEBUG"; break;
            case LogLevel::Info:    level_str = "INFO";  break;
            case LogLevel::Warning: level_str = "WARN";  break;
            case LogLevel::Error:   level_str = "ERROR"; break;
        }
        std::cout << "[" << level_str << "] " << message << std::endl;
    };
    
    try {
        // Create and start the streamer
        AirplayStreamer streamer(config);
        streamer.start();
        
        std::cout << "AirPlay server started. Waiting for connections..." << std::endl;
        std::cout << "Use Ctrl+C to stop" << std::endl;
        
        // Keep running (in a real application, you might want to handle signals)
        while (streamer.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Linking in Your Project

#### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.14)
project(my_airplay_app)

set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(FFMPEG REQUIRED)

# Add airplay_streamer as subdirectory
add_subdirectory(path/to/airplay-streamer)

# Create your executable
add_executable(my_app main.cpp)

# Link against airplay_streamer and its dependencies
target_link_libraries(my_app PRIVATE airplay_streamer)
```

## API Reference

### `Config` Structure

```cpp
struct Config {
    std::string server_name = "AirplayServer";           // AirPlay server name
    std::vector<uint8_t> hw_address = {...};             // Hardware address (MAC)
    bool low_latency = false;                            // Low latency mode
    AVFrameCallback on_video_data;                       // Video frame callback
    std::function<void(LogLevel, const char*)> log_callback; // Log callback
};
```

### `AirplayStreamer` Class

```cpp
class AirplayStreamer {
public:
    explicit AirplayStreamer(Config config);
    ~AirplayStreamer();
    
    void start();           // Start the AirPlay server
    void stop();            // Stop the server
    bool isRunning() const; // Check if server is running
};
```

### Callback Types

```cpp
// Video frame callback - called for each decoded video frame
using AVFrameCallback = std::function<void(std::shared_ptr<AVFrame>, int64_t timestamp)>;

// Log callback - called for internal logging messages
std::function<void(LogLevel level, const char* message)>
```

## Architecture

The library is built around the following components:

1. **RAOP (Remote Audio Output Protocol)**: Handles the AirPlay streaming protocol
2. **DNSSD**: Service discovery for AirPlay advertisement
3. **FFmpeg**: Video decoding and frame processing
4. **Callback System**: Asynchronous notification of events

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is licensed under the GPLv3 License - see the [LICENSE](LICENSE) file for details.

The library includes third-party components:
- **RPiPlay**: GPLv3 License
- **FFmpeg**: LGPL v2.1 or later
- **OpenSSL**: Apache License 2.0

## Troubleshooting

### Common Issues

1. **"Failed to initialize RAOP"**: Ensure all submodules are properly cloned
2. **Linker errors**: Make sure FFmpeg and OpenSSL development packages are installed
3. **No video received**: Check that the iOS device and server are on the same network

### Debugging

Enable verbose logging by setting the log callback:

```cpp
config.log_callback = [](LogLevel level, const char* message) {
    std::cerr << "[" << static_cast<int>(level) << "] " << message << std::endl;
};
```

## Acknowledgments

This project builds upon:
- [RPiPlay](https://github.com/FD-/RPiPlay) - Raspberry Pi AirPlay server implementation
- [FFmpeg](https://ffmpeg.org/) - Comprehensive multimedia framework
- [OpenSSL](https://www.openssl.org/) - Cryptography and SSL/TLS toolkit

## Support

For issues and feature requests, please [open an issue](https://github.com/your-username/airplay-streamer/issues) on GitHub.