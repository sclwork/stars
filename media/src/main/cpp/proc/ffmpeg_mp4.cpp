//
// Created by scliang on 1/14/21.
//

#include <cstdio>
#include "jni_log.h"
#include "ffmpeg_mp4.h"

#define log_d(...)  LOG_D("Media-Native:ffmpeg_mp4", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:ffmpeg_mp4", __VA_ARGS__)

namespace media {
} //namespace media

media::ffmpeg_mp4::ffmpeg_mp4(int32_t id, std::string &&n, image_args &&img, audio_args &&aud)
:_id(id), _tmp(std::string(n).replace(n.find(".mp4"), 4, "")),
name(_tmp + "_" + std::to_string(id) + ".mp4"), image(img), audio(aud),
f_rgb_name(_tmp + "_" + std::to_string(id) + ".rgb"), f_264_name(_tmp + "_" + std::to_string(id) + ".h264"),
f_pcm_name(_tmp + "_" + std::to_string(id) + ".pcm"), f_aac_name(_tmp + "_" + std::to_string(id) + ".aac") {
    image.update_frame_size();
    log_d("[%d] created. [v:%d,%d,%d,%d],[a:%d,%d,%d],\n%s\n%s\n%s\n%s\n%s.",
          _id, image.width, image.height, image.channels, image.frame_size,
          audio.channels, audio.sample_rate, audio.frame_size,
          f_rgb_name.c_str(), f_264_name.c_str(), f_pcm_name.c_str(), f_aac_name.c_str(), name.c_str());
}

media::ffmpeg_mp4::~ffmpeg_mp4() {
    log_d("[%d] release.", _id);
}

void media::ffmpeg_mp4::init() {
    av_register_all();
    avcodec_register_all();

    // reset tmp files
    std::remove(f_rgb_name.c_str());
    std::remove(f_264_name.c_str());
    std::remove(f_pcm_name.c_str());
    std::remove(f_aac_name.c_str());
    std::remove(name.c_str());
    fclose(fopen(f_rgb_name.c_str(), "wb+"));
    fclose(fopen(f_264_name.c_str(), "wb+"));
    fclose(fopen(f_pcm_name.c_str(), "wb+"));
    fclose(fopen(f_aac_name.c_str(), "wb+"));
    fclose(fopen(name.c_str(), "wb+"));
}

void media::ffmpeg_mp4::complete() {
}

void media::ffmpeg_mp4::encode_frame(std::shared_ptr<image_frame> &&img_frame,
                                     std::shared_ptr<audio_frame> &&aud_frame) {
    if (img_frame != nullptr && img_frame->available()) {
        int32_t w, h; uint32_t *data;
        img_frame->get(&w, &h, &data);
    }
    if (aud_frame != nullptr && aud_frame->available()) {
        int32_t count; uint8_t *data;
        aud_frame->get(&count, &data);
    }
}
