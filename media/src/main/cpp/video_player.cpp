//
// Created by Scliang on 4/28/21.
//

#include "jni_log.h"
#include "video_player.h"

#define log_d(...)  LOG_D("Media-Native:video_player", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:video_player", __VA_ARGS__)

namespace media {
} //namespace media

media::video_player::video_player() {
    log_d("created.");
}

media::video_player::~video_player() {
    log_d("release.");
}
