//
// Created by scliang on 1/11/21.
//

#include <string>
#include "log.h"
#include "image_cache.h"

#define log_d(...)  LOG_D("Media-Native:image_cache", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:image_cache", __VA_ARGS__)

namespace media {
} //namespace media

media::image_cache::image_cache(int32_t w, int32_t h):width(w), height(h) {
    log_d("created.");
    cache = (uint32_t *) malloc(sizeof(uint32_t) * width * height);
    if (cache == nullptr) {
        log_e("Failed malloc image cache.");
    } else {
        log_d("malloc image cache size: %d,%d.", width, height);
    }
}

media::image_cache::~image_cache() {
    if (cache) free(cache);
    log_d("release.");
}

bool media::image_cache::same_size(int32_t w, int32_t h) const {
    return w == width && h == height;
}

bool media::image_cache::available() const {
    return cache != nullptr;
}

void media::image_cache::get(int32_t *out_w, int32_t *out_h, uint32_t **out_cache) const {
    if (out_w) *out_w = width;
    if (out_h) *out_h = height;
    if (out_cache) *out_cache = cache;
}

#if DRAW_TEST_FRAME
#include <opencv2/opencv.hpp>
void media::image_cache::setup_test_data(int32_t w, int32_t h) {
    if (cache && w > 0 && h > 0) {
        // TODO: demo image path
        cv::Mat img = cv::imread("/data/user/0/com.scliang.tars/app_files/cpp.png");
        cv::cvtColor(img, img, cv::COLOR_BGRA2RGBA);
        cache = (uint32_t *) malloc(sizeof(uint32_t) * w * h);
        int32_t wof = (width - img.cols) / 2;
        int32_t hof = (height - img.rows) / 2;
        for (int32_t i = 0; i < img.rows; i++) {
            for (int32_t j = 0; j < img.cols; j++) {
                cache[(i+hof)*width+j+wof] =
                        (((int32_t)img.data[(i*img.cols+j)*4+3])<<24) +
                        (((int32_t)img.data[(i*img.cols+j)*4+2])<<16) +
                        (((int32_t)img.data[(i*img.cols+j)*4+1])<<8) +
                        (img.data[(i*img.cols+j)*4]);
            }
        }
        log_d("create test cache. %d,%d", width, height);
    }
}
#endif
