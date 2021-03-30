//
// Created by scliang on 1/11/21.
//

#include <string>
#include "jni_log.h"
#include "image_frame.h"

#define log_d(...)  LOG_D("Media-Native:image_frame", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:image_frame", __VA_ARGS__)

namespace media {
} //namespace media

media::image_frame *media::image_frame::make_default(int32_t w, int32_t h) {
    auto *frame = new image_frame(w, h);
    if (frame->available()) {
        memset(frame->cache, 0xff000000, sizeof(int32_t) * w * h);
    }
    return frame;
}

media::image_frame::image_frame(int32_t w, int32_t h)
:is_copy(false), ori(0), width(w), height(h),
cache((uint32_t*)malloc(sizeof(uint32_t)*width*height)),
op_callback(nullptr), op_ctx(nullptr) {
//    log_d("created.");
    if (cache == nullptr) {
        log_e("malloc image cache fail.");
    }
}

media::image_frame::image_frame(const image_frame &frame)
:is_copy(true), ori(frame.ori), width(frame.width), height(frame.height),
cache((uint32_t*)malloc(sizeof(uint32_t)*width*height)),
op_callback(frame.op_callback), op_ctx(nullptr) {
//    log_d("created.");
    if (cache) {
        memcpy(cache, frame.cache, sizeof(uint32_t) * width * height);
//        log_d("copy completed. %d,%d.", width, height);
    }
}

media::image_frame::~image_frame() {
    if (cache) free(cache);
//    if (!is_copy) log_d("release.");
}

bool media::image_frame::same_size(int32_t w, int32_t h) const {
    return w == width && h == height;
}

bool media::image_frame::available() const {
    return cache != nullptr;
}

void media::image_frame::set_ori(int32_t o) {
    ori = o;
}

bool media::image_frame::use_mirror() const {
    return ori == 270;
}

void media::image_frame::get(int32_t *out_w, int32_t *out_h, uint32_t **out_cache) const {
    if (out_w) *out_w = width;
    if (out_h) *out_h = height;
    if (out_cache) *out_cache = cache;
}

void media::image_frame::set_op_callback(IMAGE_FRAME_OP_CALLBACK cb, void *ctx) {
    op_callback = cb;
    op_ctx = ctx;
}

void media::image_frame::run_op_callback(image_frame *frame) {
    if (op_callback && frame) {
        auto *ctx = new image_frame_ctx();
        ctx->frame = frame;
        ctx->ctx = op_ctx;
        op_callback(ctx);
        delete ctx;
    }
}

#if DRAW_TEST_FRAME
#include <opencv2/opencv.hpp>
void media::image_frame::setup_test_data(int32_t w, int32_t h) {
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
