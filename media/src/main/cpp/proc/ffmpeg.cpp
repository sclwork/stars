//
// Created by scliang on 1/14/21.
//

#include <cstdio>
#include "log.h"
#include "loop/loop.h"
#include "ffmpeg.h"

#define log_d(...)  LOG_D("Media-Native:ffmpeg", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:ffmpeg", __VA_ARGS__)

// PCM Size=采样率*采样时间*采样位深/8*通道数（Bytes）

namespace media {

class video_start_ctx {
public:
    explicit video_start_ctx(std::string &&n,
                             ff_image_args &&img,
                             ff_audio_args &&aud,
                             std::shared_ptr<ffmpeg> &&f)
                                :name(n), image(img), audio(aud), ffmpeg(f) {}
    ~video_start_ctx() = default;

public:
    void run() {
        if (ffmpeg != nullptr) {
            ffmpeg->video_encode_start(std::forward<std::string>(name),
                                       std::forward<ff_image_args>(image),
                                       std::forward<ff_audio_args>(audio));
        }
    }

private:
    std::string             name;
    ff_image_args           image;
    ff_audio_args           audio;
    std::shared_ptr<ffmpeg> ffmpeg;
};

class video_stop_ctx {
public:
    explicit video_stop_ctx(std::shared_ptr<ffmpeg> &&f):ffmpeg(f) {}
    ~video_stop_ctx() = default;

public:
    void run() {
        if (ffmpeg != nullptr) {
            ffmpeg->video_encode_stop();
        }
    }

private:
    std::shared_ptr<ffmpeg> ffmpeg;
};

class video_encode_ctx {
public:
    explicit video_encode_ctx(std::shared_ptr<ffmpeg>      &&f,
                              std::shared_ptr<image_frame> &&img,
                              std::shared_ptr<audio_frame> &&aud)
                                :ffmpeg(f), img_frame(img), aud_frame(aud) {}

    ~video_encode_ctx() = default;

public:
    void run() {
        if (ffmpeg != nullptr) {
            ffmpeg->video_encode_frame(std::shared_ptr<image_frame>(img_frame),
                                       std::shared_ptr<audio_frame>(aud_frame));
        }
    }

private:
    std::shared_ptr<ffmpeg>      ffmpeg;
    std::shared_ptr<image_frame> img_frame;
    std::shared_ptr<audio_frame> aud_frame;
};

} //namespace media

/*
 * run in media collect thread.
 */
void media::ffmpeg::video_encode_frame(std::string                  &&n,
                                       ff_image_args                &&img,
                                       ff_audio_args                &&aud,
                                       std::shared_ptr<ffmpeg>      &&ff,
                                       std::shared_ptr<image_frame> &&img_frame,
                                       std::shared_ptr<audio_frame> &&aud_frame) {
    if (!ff->video_encode_available()) {
        auto *ctx = new video_start_ctx(std::forward<std::string>(n),
                                        std::forward<ff_image_args>(img),
                                        std::forward<ff_audio_args>(aud),
                                        std::forward<std::shared_ptr<ffmpeg>>(ff));
        loop_post_encode([](void *ctx, void(*callback)(void*)) {
            ((video_start_ctx*)ctx)->run();
            delete ((video_start_ctx*)ctx);
        }, ctx, nullptr);
    }
    auto *ctx = new video_encode_ctx(std::forward<std::shared_ptr<ffmpeg>>(ff),
                                     std::forward<std::shared_ptr<image_frame>>(img_frame),
                                     std::forward<std::shared_ptr<audio_frame>>(aud_frame));
    loop_post_encode([](void *ctx, void(*callback)(void*)) {
        ((video_encode_ctx*)ctx)->run();
        delete ((video_encode_ctx*)ctx);
    }, ctx, nullptr);
}

/*
 * run in media collect thread.
 */
void media::ffmpeg::video_encode_idle(std::shared_ptr<ffmpeg> &&ff) {
    if (ff->video_encode_available()) {
        auto *ctx = new video_stop_ctx(std::forward<std::shared_ptr<ffmpeg>>(ff));
        loop_post_encode([](void *ctx, void(*callback)(void *)) {
            ((video_stop_ctx *) ctx)->run();
            delete ((video_stop_ctx *) ctx);
        }, ctx, nullptr);
    }
}

media::ffmpeg::ffmpeg():mp4(nullptr), mp4_mutex() {
    log_d("created.");
}

media::ffmpeg::~ffmpeg() {
    log_d("release.");
}

/*
 * run in media collect thread
 */
bool media::ffmpeg::video_encode_available() const {
    std::lock_guard<std::mutex> lock(mp4_mutex);
    return mp4 != nullptr;
}

/*
 * run in media encode thread
 */
void media::ffmpeg::video_encode_start(std::string   &&name,
                                       ff_image_args &&image_args,
                                       ff_audio_args &&audio_args) {
    std::lock_guard<std::mutex> lock(mp4_mutex);
    if (mp4 == nullptr) {
        mp4 = std::make_shared<media::ffmpeg_mp4>(std::forward<std::string>(name),
                                                  std::forward<ff_image_args>(image_args),
                                                  std::forward<ff_audio_args>(audio_args));
        mp4->reset_tmp_files();
        mp4->init_image_encode();
        mp4->init_audio_encode();
    }
}

/*
 * run in media encode thread
 */
void media::ffmpeg::video_encode_stop() {
    std::lock_guard<std::mutex> lock(mp4_mutex);
    if (mp4 != nullptr) {
        mp4->close_image_encode();
        mp4->close_audio_encode();
        mp4.reset();
    }
}

/*
 * run in media encode thread
 */
void media::ffmpeg::video_encode_frame(std::shared_ptr<image_frame> &&img_frame,
                                       std::shared_ptr<audio_frame> &&aud_frame) {
    std::lock_guard<std::mutex> lock(mp4_mutex);
    if (mp4 != nullptr) {
        mp4->append_av_frame(std::forward<std::shared_ptr<image_frame>>(img_frame),
                             std::forward<std::shared_ptr<audio_frame>>(aud_frame));
    }
}
