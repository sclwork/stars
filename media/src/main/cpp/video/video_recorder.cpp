//
// Created by Scliang on 2/7/21.
//

#include "log.h"
#include "video_recorder.h"

#define log_d(...)  LOG_D("Media-Native:video_recorder", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:video_recorder", __VA_ARGS__)

media::video_recorder::video_recorder() {
    log_d("created.");
}

media::video_recorder::~video_recorder() {
    log_d("release.");
}

void media::video_recorder::start_preview(int32_t w, int32_t h,
                                          void (*callback)(std::shared_ptr<image_frame>&)) {

}

void media::video_recorder::stop_preview() {
}
