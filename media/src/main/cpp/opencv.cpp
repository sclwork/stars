//
// Created by Scliang on 3/22/21.
//

#include "jni_log.h"
#include "opencv.h"

#define log_d(...)  LOG_D("Media-Native:opencv", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:opencv", __VA_ARGS__)

namespace media {
} //namespace media

media::opencv::opencv() {
    log_d("created.");
}

media::opencv::~opencv() {
    log_d("release.");
}

void media::opencv::grey_frame(const image_frame &frame) {
    int32_t width, height; uint32_t *data;
    frame.get(&width, &height, &data);
    cv::Mat img(height, width, CV_8UC4, data);
    cv::cvtColor(img, img, cv::COLOR_BGRA2GRAY);
    cv::cvtColor(img, img, cv::COLOR_GRAY2BGRA);
    memcpy(data, img.data, sizeof(uint32_t) * width * height);
}
