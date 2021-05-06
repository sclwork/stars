//
// Created by Scliang on 4/28/21.
//

#include <opencv2/opencv.hpp>
#include "utils.h"
#include "jni_log.h"
#include "video_player.h"
#include "image_frame.h"

#define log_d(...)  LOG_D("Media-Native:video_player", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:video_player", __VA_ARGS__)

namespace media {

static void img_decode_run(video_decoder *dp) {
    log_d("decode started.");
    while (dp->running()) dp->run();
    log_d("decode stoped ...");
    delete dp;
}

} //namespace media

media::video_player::video_player()
:plying(false), decoder(nullptr) {
    log_d("created.");
}

media::video_player::~video_player() {
    stop_play();
    log_d("release.");
}

void media::video_player::start_play(int32_t w, int32_t h, std::string &&name,
                                     void (*callback)(image_frame&&)) {
    log_d("start video play: %s.", name.c_str());
    if (decoder != nullptr) {
        decoder->unrunning();
        decoder = nullptr;
    }
    auto runnable = std::make_shared<std::atomic_bool>(true);
    auto *ctx = new video_decoder(w, h, std::forward<std::string>(name), runnable, callback);
    decoder = std::shared_ptr<video_decoder>(ctx, [](void*){}); // delete dp in img_decode_run
    std::thread encode_t(img_decode_run, ctx);
    encode_t.detach();
    plying = true;
}

void media::video_player::stop_play() {
    log_d("stop video play.");
    plying = false;
    if (decoder != nullptr) {
        decoder->unrunning();
        decoder = nullptr;
    }
}
