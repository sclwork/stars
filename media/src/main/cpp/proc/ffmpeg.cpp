//
// Created by scliang on 1/14/21.
//

#include "log.h"
#include "loop/loop.h"
#include "ffmpeg.h"

#define log_d(...)  LOG_D("Media-Native:ffmpeg", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:ffmpeg", __VA_ARGS__)

namespace media {

class video_encode_ctx {
public:
    video_encode_ctx(std::shared_ptr<ffmpeg> &&f,
                     std::shared_ptr<image_frame> &&img,
                     std::shared_ptr<audio_frame> &&aud)
                     :ffmpeg(f), img_frame(img), aud_frame(aud) {}
    ~video_encode_ctx() = default;

public:
    void run() {
//        log_d("aud: %d.", ((std::shared_ptr<int16_t>)aud_frame->get(nullptr)).get()[0]);
    }

private:
    std::shared_ptr<ffmpeg>      ffmpeg;
    std::shared_ptr<image_frame> img_frame;
    std::shared_ptr<audio_frame> aud_frame;
};

} //namespace media

media::ffmpeg::ffmpeg() {
    log_d("created.");
}

media::ffmpeg::~ffmpeg() {
    log_d("release.");
}

void media::ffmpeg::video_encode_start(std::string &name) {
}

void media::ffmpeg::video_encode_stop() {
}

void media::ffmpeg::video_encode_frame(std::shared_ptr<image_frame> &&img_frame,
                                       std::shared_ptr<audio_frame> &&aud_frame) {
//    auto *ctx = new video_encode_ctx(std::shared_ptr<ffmpeg>(this),
//            std::forward<std::shared_ptr<image_frame>>(img_frame),
//            std::forward<std::shared_ptr<audio_frame>>(aud_frame));
//    loop_post([](void *ctx, void(*callback)(void*)) {
//        ((video_encode_ctx*)ctx)->run();
//        delete ((video_encode_ctx*)ctx);
//    }, ctx, nullptr);
}
