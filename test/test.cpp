// test/test.cpp
#include <airplay_streamer.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>
#include <fstream>
extern "C" {
#include <libavutil/frame.h>
}

using namespace ender::airplay_streamer;

// 全局变量用于信号处理
static bool g_running = true;

// 信号处理函数
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", stopping..." << std::endl;
    g_running = false;
}

// 保存帧到文件的函数（用于调试）
void saveFrameToFile(const std::shared_ptr<AVFrame>& frame, int64_t timestamp, int frame_count) {
    std::string filename = "frame_" + std::to_string(frame_count) + "_" + std::to_string(timestamp) + ".raw";
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
        // 保存YUV数据
        for (int i = 0; i < frame->height; ++i) {
            file.write(reinterpret_cast<const char*>(frame->data[0] + i * frame->linesize[0]), frame->width);
        }
        for (int i = 0; i < frame->height/2; ++i) {
            file.write(reinterpret_cast<const char*>(frame->data[1] + i * frame->linesize[1]), frame->width/2);
        }
        for (int i = 0; i < frame->height/2; ++i) {
            file.write(reinterpret_cast<const char*>(frame->data[2] + i * frame->linesize[2]), frame->width/2);
        }
        file.close();
        std::cout << "Saved frame to " << filename << std::endl;
    }
}

int main() {
    std::cout << "=== AirPlay Streamer Test ===" << std::endl;

    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        // 配置AirPlay流媒体
        Config config;
        config.server_name = "AirPlayTestServer";
        config.hw_address = {0x48, 0x5D, 0x60, 0x7C, 0xEE, 0x22};

        // 帧计数器
        int frame_count = 0;
        auto start_time = std::chrono::high_resolution_clock::now();

        // 设置视频数据回调
        config.on_video_data = [&frame_count, start_time](std::shared_ptr<AVFrame> frame, int64_t timestamp) {
            if (frame) {
                frame_count++;

                // 每100帧打印一次统计信息
                if (frame_count % 100 == 0) {
                    auto current_time = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);

                    std::cout << "[INFO] Received " << frame_count << " frames in "
                              << duration.count() << "ms ("
                              << (frame_count * 1000.0 / duration.count()) << " FPS)" << std::endl;
                    std::cout << "[INFO] Frame size: " << frame->width << "x" << frame->height
                              << ", format: " << frame->format << std::endl;
                }

                // 可选：保存前几帧到文件进行调试
                if (frame_count <= 5) {
                    // saveFrameToFile(frame, timestamp, frame_count);
                }
            }
        };

        // 设置日志回调
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

        std::cout << "Initializing AirPlay streamer..." << std::endl;

        // 创建并启动流媒体服务
        AirplayStreamer streamer(config);
        streamer.start();

        std::cout << "✅ AirPlay server started successfully!" << std::endl;
        std::cout << "📡 Server name: " << config.server_name << std::endl;
        std::cout << "📱 To connect:" << std::endl;
        std::cout << "   1. On your iOS device, open Control Center" << std::endl;
        std::cout << "   2. Press and hold the AirPlay/Audio button" << std::endl;
        std::cout << "   3. Select '" << config.server_name << "' from the list" << std::endl;
        std::cout << "   4. Start mirroring your screen" << std::endl;
        std::cout << " Press Ctrl+C to stop the server" << std::endl;
        std::cout << "=========================================" << std::endl;

        // 主循环
        while (g_running && streamer.isRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        std::cout << "\n Shutting down AirPlay server..." << std::endl;

        // 停止服务
        streamer.stop();

        // 打印最终统计
        auto end_time = std::chrono::high_resolution_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << "📊 Final statistics:" << std::endl;
        std::cout << "   - Total frames received: " << frame_count << std::endl;
        std::cout << "   - Total duration: " << total_duration.count() << "ms" << std::endl;
        if (total_duration.count() > 0) {
            std::cout << "   - Average FPS: " << (frame_count * 1000.0 / total_duration.count()) << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Unknown error occurred" << std::endl;
        return 1;
    }

    std::cout << "✅ Test completed successfully!" << std::endl;
    return 0;
}
