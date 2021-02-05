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
void media::ffmpeg::video_encode_frame(std::shared_ptr<ffmpeg>      &&ff,
                                       std::shared_ptr<image_frame> &&img_frame,
                                       std::shared_ptr<audio_frame> &&aud_frame) {
    auto *ctx = new video_encode_ctx(std::shared_ptr<ffmpeg>(ff),
                                     std::shared_ptr<image_frame>(img_frame),
                                     std::shared_ptr<audio_frame>(aud_frame));
    loop_post_encode([](void *ctx, void(*callback)(void*)) {
        ((video_encode_ctx*)ctx)->run();
        delete ((video_encode_ctx*)ctx);
    }, ctx, nullptr);
}

namespace media {

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

} //namespace media

/*
 * run in renderer thread.
 */
void media::ffmpeg::video_encode_stop(std::shared_ptr<ffmpeg> &&ff) {
    auto *ctx = new video_stop_ctx(std::shared_ptr<ffmpeg>(ff));
    loop_post_encode([](void *ctx, void(*callback)(void *)) {
        ((video_stop_ctx *) ctx)->run();
        delete ((video_stop_ctx *) ctx);
    }, ctx, nullptr);
}

media::ffmpeg::ffmpeg():mp4_arr(), mp4_thrd_ids(), mp4_name(), mp4_img_args(), mp4_aud_args() {
    log_d("created.");
}

media::ffmpeg::~ffmpeg() {
    log_d("release.");
}

void media::ffmpeg::set_video_name(std::string &&n) {
    mp4_name = n;
}

void media::ffmpeg::set_video_image_args(ff_image_args &&img) {
    mp4_img_args = img;
}

void media::ffmpeg::set_video_audio_args(ff_audio_args &&aud) {
    mp4_aud_args = aud;
}

namespace media {

class video_complete_ctx {
public:
    explicit video_complete_ctx(ffmpeg *f):ff(f) {}
    ~video_complete_ctx() = default;

public:
    void run() {
        if (ff != nullptr) {
            ff->video_encode_complete();
        }
    }

private:
    ffmpeg *ff;
};

} //namespace media

/*
 * run in media encode thread
 */
void media::ffmpeg::video_encode_stop() {
    for (auto &mp4 : mp4_arr) {
        if (mp4 != nullptr) {
            mp4->request_stop();
        }
    }
    auto *ctx = new video_complete_ctx(this);
    loop_post_encode([](void *ctx, void(*callback)(void *)) {
        ((video_complete_ctx *) ctx)->run();
        delete ((video_complete_ctx *) ctx);
    }, ctx, nullptr);
}

/*
 * run in media encode thread
 */
void media::ffmpeg::video_encode_frame(std::shared_ptr<image_frame> &&img_frame,
                                       std::shared_ptr<audio_frame> &&aud_frame) {
    std::__thread_id z;
    std::__thread_id id = std::this_thread::get_id();
    int mi = -1;
    for (int32_t i = 0; i < 3; ++i) {
        if (z == mp4_thrd_ids[i]) {
            mp4_thrd_ids[i] = id;
            mi = i;
            break;
        } else if (id == mp4_thrd_ids[i]) {
            mi = i;
            break;
        }
    }
    if (mi >= 0 && mp4_arr[mi] == nullptr) {
        mp4_arr[mi] = std::make_shared<media::ffmpeg_mp4>(mi,
                                                          std::forward<std::string>(mp4_name),
                                                          std::forward<ff_image_args>(mp4_img_args),
                                                          std::forward<ff_audio_args>(mp4_aud_args));
        mp4_arr[mi]->reset_tmp_files();
        mp4_arr[mi]->init_image_encode();
        mp4_arr[mi]->init_audio_encode();
    }
    if (mi >= 0 && mp4_arr[mi] != nullptr) {
        mp4_arr[mi]->encode_frame(std::shared_ptr<image_frame>(img_frame),
                                  std::shared_ptr<audio_frame>(aud_frame));
    }
}

/*
 * run in media encode thread
 */
void media::ffmpeg::video_encode_complete() {
    for (auto &mp4 : mp4_arr) {
        if (mp4 != nullptr && mp4->check_req_stop()) {
            mp4.reset();
        }
    }
}
