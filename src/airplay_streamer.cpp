// src/airplay_streamer.cpp

#include <airplay_streamer.hpp>

extern "C" {
#include "raop.h"
#include "dnssd.h"
}

#include <memory>
#include <stdexcept>
#include <cstring>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
}

namespace ender::airplay_streamer {
    class AirplayStreamer::Impl {
    public:
        Config config;

        bool running = false;
        raop_t *raop = nullptr;
        dnssd_t *dnssd = nullptr;

        AVCodecContext *video_decoder = nullptr;
        SwsContext *sws_context = nullptr;

        void initCodec() {
            const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
            if (!codec) {
                throw std::runtime_error("Cannot find H.264 decoder.");
            }

            video_decoder = avcodec_alloc_context3(codec);
            if (!video_decoder) {
                throw std::runtime_error("Failed to allocate video decoder context.");
            }

            if (avcodec_open2(video_decoder, codec, nullptr) < 0) {
                throw std::runtime_error("Failed to open H.264 decoder.");
            }
        }
    };

    AirplayStreamer::AirplayStreamer(const Config &config) : impl_(std::make_unique<Impl>()) {
        impl_->config = config;

        impl_->initCodec();
        av_log_set_level(AV_LOG_INFO);

        // 初始化底层 Airplay
        raop_callbacks_t callbacks{};
        callbacks.cls = this; // 设置回调类指针

        callbacks.conn_init = [](void *cls) {
            // 连接初始化回调
        };

        callbacks.conn_destroy = [](void *cls) {
            // 连接销毁回调
        };

        callbacks.video_process = [](void *cls, raop_ntp_t *ntp, h264_decode_struct *data) {
            auto *streamer = static_cast<AirplayStreamer *>(cls);

            if (streamer && data && data->data && data->data_len > 0) {
                AVPacket *pkt = av_packet_alloc();
                if (!pkt) return;

                pkt->data = static_cast<uint8_t *>(data->data);
                pkt->size = data->data_len;
                pkt->pts = data->pts;

                AVFrame *frame = av_frame_alloc();
                if (!frame) {
                    av_packet_free(&pkt);
                    return;
                }

                int ret = avcodec_send_packet(streamer->impl_->video_decoder, pkt);
                if (ret == 0) {
                    while (avcodec_receive_frame(streamer->impl_->video_decoder, frame) == 0) {
                        // 创建智能指针包装
                        auto frame_deleter = [](AVFrame *f) {
                            if (f) av_frame_free(&f);
                        };
                        auto shared_frame = std::shared_ptr<AVFrame>(av_frame_clone(frame), frame_deleter);

                        if (streamer->impl_->config.on_video_data) {
                            streamer->impl_->config.on_video_data(shared_frame, data->pts);
                        }
                    }
                }

                av_frame_free(&frame);
                av_packet_free(&pkt);
            }
        };

        // 注意：原 raop.h 中没有 log_callback 成员，使用 raop_set_log_callback 替代
        impl_->raop = raop_init(10, &callbacks);
        if (!impl_->raop) {
            throw std::runtime_error("Failed to initialize RAOP");
        }

        // 设置日志回调（正确的方式）
        if (impl_->config.log_callback) {
            raop_set_log_callback(impl_->raop, [](void *cls, int level, const char *msg) {
                auto *streamer = static_cast<AirplayStreamer *>(cls);
                if (streamer && streamer->impl_->config.log_callback) {
                    LogLevel log_level;
                    switch (level) {
                        case 0:
                        case 1:
                        case 2:
                        case 3: log_level = LogLevel::Error;
                            break;
                        case 4: log_level = LogLevel::Warning;
                            break;
                        case 5:
                        case 6: log_level = LogLevel::Info;
                            break;
                        case 7: log_level = LogLevel::Debug;
                            break;
                        default: log_level = LogLevel::Info;
                            break;
                    }
                    streamer->impl_->config.log_callback(log_level, msg);
                }
            }, this);
        }

        // 注册 DNSSD - 使用正确的函数名
        int dnssd_error = 0;
        impl_->dnssd = dnssd_init(impl_->config.server_name.c_str(),
                                  impl_->config.server_name.length(),
                                  reinterpret_cast<const char *>(impl_->config.hw_address.data()),
                                  impl_->config.hw_address.size(),
                                  &dnssd_error);

        if (!impl_->dnssd || dnssd_error != 0) {
            throw std::runtime_error("Failed to init DNSSD, error: " + std::to_string(dnssd_error));
        }

        raop_set_dnssd(impl_->raop, impl_->dnssd);

        unsigned short port = 0;
        if (raop_start(impl_->raop, &port) != 0) {
            throw std::runtime_error("Failed to start RAOP");
        }

        if (dnssd_register_raop(impl_->dnssd, port) != 0) {
            throw std::runtime_error("Failed to register RAOP service");
        }

        if (dnssd_register_airplay(impl_->dnssd, port + 1) != 0) {
            throw std::runtime_error("Failed to register AirPlay service");
        }
    }

    AirplayStreamer::~AirplayStreamer() {
        if (impl_) {
            if (impl_->raop) {
                raop_stop(impl_->raop);
                raop_destroy(impl_->raop);
                impl_->raop = nullptr;
            }

            if (impl_->dnssd) {
                dnssd_unregister_raop(impl_->dnssd);
                dnssd_unregister_airplay(impl_->dnssd);
                dnssd_destroy(impl_->dnssd);
                impl_->dnssd = nullptr;
            }

            if (impl_->video_decoder) {
                avcodec_free_context(&impl_->video_decoder);
            }
        }
    }

    void AirplayStreamer::start() const {
        impl_->running = true;
    }

    void AirplayStreamer::stop() const {
        impl_->running = false;
    }

    bool AirplayStreamer::isRunning() const {
        return impl_->running && impl_->raop && raop_is_running(impl_->raop);
    }
} // namespace airplay_streamer
