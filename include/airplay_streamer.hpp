// include/airplay_streamer.hpp
#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

struct AVFrame;

namespace airplay_streamer {

    enum class LogLevel {
        Debug,
        Info,
        Warning,
        Error
    };

    using AVFrameCallback = std::function<void(std::shared_ptr<AVFrame>, int64_t timestamp)>;

    struct Config {
        std::string server_name = "AirplayServer";
        std::vector<uint8_t> hw_address = {0x48, 0x5D, 0x60, 0x7C, 0xEE, 0x22};

        bool low_latency = false;

        AVFrameCallback on_video_data;
        AVFrameCallback on_audio_data; // 可选，如果要用解码后的 AAC 播放的话

        std::function<void(LogLevel level, const char*)> log_callback = nullptr;
    };

    class AirplayStreamer {
    public:
        explicit AirplayStreamer(Config config);

        ~AirplayStreamer();

        // 启动服务线程
        void start();

        // 停止服务
        void stop();

        // 是否正在运行中
        bool isRunning() const;

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };

} // namespace airplay_streamer
