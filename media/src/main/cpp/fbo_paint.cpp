//
// Created by Scliang on 3/30/21.
//

#include "jni_log.h"
#include "fbo_paint.h"

#define log_d(...)  LOG_D("Media-Native:fbo_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:fbo_paint", __VA_ARGS__)

namespace media {
} //namespace media

media::fbo_paint::fbo_paint()
:cvs_width(0), cvs_height(0), cvs_ratio(0) {
    log_d("created.");
}

media::fbo_paint::~fbo_paint() {
    log_d("release.");
}

void media::fbo_paint::set_canvas_size(int32_t width, int32_t height) {

    cvs_width = width;
    cvs_height = height;
    cvs_ratio = (float)width/(float)height;
//    glViewport(0, 0, cvs_width, cvs_height);
//    glClearColor(0.0, 0.0, 0.0, 1.0);

    log_d("canvas size: %d,%d %0.4f", cvs_width, cvs_height, cvs_ratio);
}

void media::fbo_paint::draw(const image_frame &frame, image_frame &of) {
    if (!frame.available()) {
        return;
    }

    of = frame;
}
