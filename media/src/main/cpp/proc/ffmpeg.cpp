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
} //namespace media

media::ffmpeg::ffmpeg() {
    log_d("created.");
}

media::ffmpeg::~ffmpeg() {
    log_d("release.");
}
