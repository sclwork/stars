//
// Created by Scliang on 3/8/21.
//

#include "log.h"
#include "ffmpeg_rtmp.h"

#define log_d(...)  LOG_D("Media-Native:ffmpeg_rtmp", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:ffmpeg_rtmp", __VA_ARGS__)

namespace media {
} //namespace media

media::ffmpeg_rtmp::ffmpeg_rtmp(int32_t id, std::string &&n, image_args &&img, audio_args &&aud)
:_id(id), name(n), image(img), audio(aud) {
    image.update_frame_size();
    log_d("[%d] created. [v:%d,%d,%d,%d],[a:%d,%d,%d].",
          _id, image.width, image.height, image.channels, image.frame_size,
          audio.channels, audio.sample_rate, audio.frame_size);
}

media::ffmpeg_rtmp::~ffmpeg_rtmp() {
    log_d("[%d] release.", _id);
}

void media::ffmpeg_rtmp::init() {
}

void media::ffmpeg_rtmp::complete() {
}

void media::ffmpeg_rtmp::encode_frame(std::shared_ptr<image_frame> &&img_frame,
                                      std::shared_ptr<audio_frame> &&aud_frame) {
}
