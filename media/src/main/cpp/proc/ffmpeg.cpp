//
// Created by scliang on 1/14/21.
//

#include "log.h"
#include "ffmpeg.h"

#define log_d(...)  LOG_D("Media-Native:ffmpeg", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:ffmpeg", __VA_ARGS__)

namespace media {
} //namespace media

media::ffmpeg::ffmpeg() {
    log_d("created.");
}

media::ffmpeg::~ffmpeg() {
    log_d("release.");
}

void media::ffmpeg::encode_frame(std::shared_ptr<image_frame> &&frame) {
}
