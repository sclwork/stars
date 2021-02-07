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
