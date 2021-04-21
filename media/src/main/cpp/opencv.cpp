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

uint32_t *media::opencv::load_image(const std::string &file, int32_t *ow, int32_t *oh) {
    cv::Mat img = cv::imread(file);
    if (img.empty()) {
        if (ow != nullptr) *ow = 0;
        if (oh != nullptr) *oh = 0;
        return nullptr;
    }

    log_d("load_image channels: %d.", img.channels());
    if (img.channels() == 3) {
        cv::cvtColor(img, img, cv::COLOR_BGR2RGBA);
    } else if (img.channels() == 4) {
        cv::cvtColor(img, img, cv::COLOR_BGRA2RGBA);
    }

    int32_t w = img.cols;
    int32_t h = img.rows;
    auto *data = (uint32_t *)malloc(sizeof(uint32_t) * w * h);
    if (data == nullptr) {
        if (ow != nullptr) *ow = 0;
        if (oh != nullptr) *oh = 0;
        return nullptr;
    }

    if (ow != nullptr) *ow = w;
    if (oh != nullptr) *oh = h;
    memcpy(data, img.data, sizeof(uint32_t) * w * h);
    return data;
}

void media::opencv::grey_data(int32_t width, int32_t height, uint32_t *data) {
    cv::Mat img(height, width, CV_8UC4, data);
    cv::cvtColor(img, img, cv::COLOR_BGRA2GRAY);
    cv::cvtColor(img, img, cv::COLOR_GRAY2BGRA);
    memcpy(data, img.data, sizeof(uint32_t) * width * height);
}

void media::opencv::grey_frame(const image_frame &frame) {
    int32_t width, height; uint32_t *data;
    frame.get(&width, &height, &data);
    cv::Mat img(height, width, CV_8UC4, data);
    cv::cvtColor(img, img, cv::COLOR_BGRA2GRAY);
    cv::cvtColor(img, img, cv::COLOR_GRAY2BGRA);
    memcpy(data, img.data, sizeof(uint32_t) * width * height);
}
