//
// Created by Scliang on 3/8/21.
//

#include "proc.h"
#include "jni_log.h"
#include "ffmpeg_rtmp.h"

#define log_d(...)  LOG_D("Media-Native:ffmpeg_rtmp", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:ffmpeg_rtmp", __VA_ARGS__)

#define HAVE_IMAGE_STREAM
#define HAVE_AUDIO_STREAM

namespace media {
} //namespace media

media::ffmpeg_rtmp::ffmpeg_rtmp(std::string &&f, std::string &&n, image_args &&img, audio_args &&aud)
:ffmpeg_h264(std::forward<image_args>(img), std::forward<audio_args>(aud)), file(f), name(n) {
    log_d("created. [v:%d,%d,%d,%d],[a:%d,%d,%d].",
          image.width, image.height, image.channels, image.frame_size,
          audio.channels, audio.sample_rate, audio.frame_size);
}

media::ffmpeg_rtmp::~ffmpeg_rtmp() {
    log_d("release.");
}

void media::ffmpeg_rtmp::init() {
    av_register_all();
    avcodec_register_all();
    avformat_network_init();
    on_init_all("flv", name);
}
