//
// Created by scliang on 1/14/21.
//

#include "jni_log.h"
#include "tflite.h"

#define log_d(...)  LOG_D("Media-Native:tflite", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:tflite", __VA_ARGS__)

namespace media {
} //namespace media

media::tflite::tflite() {
    log_d("created. v%s", TfLiteVersion());
}

media::tflite::~tflite() {
    log_d("release.");
}
