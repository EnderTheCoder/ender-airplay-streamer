// src/airplay_streamer.cpp

#include <airplay_streamer.hpp>

extern "C" {
#include "raop.h"
#include "dnssd.h"
#include "logger.h"
#include "mirror_buffer.h"
}

#include <memory>
#include <stdexcept>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
}

namespace airplay_streamer {

struct AirplayStreamer::Impl {
    Config config;

    bool running = false;
    raop_t* raop = nullptr;
    dnssd_t* dnssd = nullptr;

    AVCodecContext* video_decoder = nullptr;
    SwsContext* sws_context = nullptr;

    void initCodec() {
        const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
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

AirplayStreamer::AirplayStreamer(Config config) : impl_(std::make_unique<Impl>()) {
    impl_->config = config;

    impl_->initCodec();
    av_log_set_level(AV_LOG_INFO);

    // 初始化底层 Airplay
    raop_callbacks_t callbacks{};
    callbacks.conn_init = [](void*) {};
    callbacks.conn_destroy = [](void*) {};
    callbacks.video_process = [](void* cls, raop_ntp_t*, h264_decode_struct* data) {
        auto* streamer = static_cast<AirplayStreamer*>(cls);

        AVPacket* pkt = av_packet_alloc();
        pkt->data = static_cast<uint8_t*>(data->data);
        pkt->size = data->data_len;

        AVFrame* frame = av_frame_alloc();
        int ret = avcodec_send_packet(streamer->impl_->video_decoder, pkt);
        if (ret == 0) {
            while (avcodec_receive_frame(streamer->impl_->video_decoder, frame) == 0) {
                auto shared_frame = std::shared_ptr<AVFrame>(av_frame_clone(frame), av_frame_free);
                if (streamer->impl_->config.on_video_data) {
                    streamer->impl_->config.on_video_data(shared_frame, data->pts);
                } else {
                    av_frame_free(&frame);
                }
            }
        }
        av_packet_free(&pkt);
    };
    callbacks.log_callback = [](void* cls, int level, const char* msg) {
        auto* streamer = static_cast<AirplayStreamer*>(cls);
        LogLevel log_level = static_cast<LogLevel>(level);
        if (streamer->impl_->config.log_callback) {
            streamer->impl_->config.log_callback(log_level, msg);
        }
    };

    impl_->raop = raop_init_with_mirror(10, &callbacks);
    if (!impl_->raop) {
        throw std::runtime_error("Failed to initialize RAOP");
    }

    // 注册 DNSSD
    impl_->dnssd = dnssd_init_with_name(config.server_name.c_str(), strlen(config.server_name.c_str()),
                                       config.hw_address.data(), config.hw_address.size());

    if (!impl_->dnssd) {
        throw std::runtime_error("Failed to init DNSSD");
    }

    raop_set_dnssd(impl_->raop, impl_->dnssd);

    unsigned short port = 0;
    raop_start(impl_->raop, &port);
    dnssd_register_raop(impl_->dnssd, port);
    dnssd_register_airplay(impl_->dnssd, port + 1);
}

AirplayStreamer::~AirplayStreamer() {
    if (impl_) {
        if (impl_->raop) {
            raop_destroy(impl_->raop);
        }
        if (impl_->dnssd) {
            dnssd_unregister_raop(impl_->dnssd);
            dnssd_unregister_airplay(impl_->dnssd);
            dnssd_destroy(impl_->dnssd);
        }

        avcodec_free_context(&impl_->video_decoder);
    }
}

void AirplayStreamer::start() {
    impl_->running = true;
}

void AirplayStreamer::stop() {
    impl_->running = false;
}

bool AirplayStreamer::isRunning() const {
    return impl_->running;
}

} // namespace airplay_streamer
