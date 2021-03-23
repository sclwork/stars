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

media::ffmpeg_mp4::ffmpeg_mp4(std::string &&n, image_args &&img, audio_args &&aud)
:ffmpeg_h264(std::forward<image_args>(img), std::forward<audio_args>(aud)),
_tmp(std::string(n).replace(n.find(".mp4"), 4, "")),
name(_tmp + ".mp4"), f_rgb_name(_tmp + ".rgb"), f_264_name(_tmp + ".h264"),
f_pcm_name(_tmp + ".pcm"), f_aac_name(_tmp + ".aac") {
    log_d("created. [v:%d,%d,%d,%d],[a:%d,%d,%d],\n%s,\n%s,\n%s,\n%s,\n%s.",
          image.width, image.height, image.channels, image.frame_size,
          audio.channels, audio.sample_rate, audio.frame_size,
          f_rgb_name.c_str(), f_264_name.c_str(), f_pcm_name.c_str(), f_aac_name.c_str(), name.c_str());
}

media::ffmpeg_mp4::~ffmpeg_mp4() {
    log_d("release.");
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

#ifdef HAVE_IMAGE_STREAM
    std::string out_name(name);
#else
    std::string out_name(f_aac_name);
#endif

    on_init_all("", out_name);
}
