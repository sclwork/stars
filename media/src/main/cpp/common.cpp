//
// Created by scliang on 1/6/21.
//

#include "common.h"
#include "video/image_renderer.h"

#define log_d(...)  LOG_D("Media-Native:common", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:common", __VA_ARGS__)

namespace media {
} //namespace media

void media::frame::draw_to_renderer(renderer *renderer) {
    if (data && renderer) {
        renderer->draw_frame(width, height, data);
    }
}

#if LOG_ABLE
#include <opencv2/opencv.hpp>
void media::frame::setup_test_data(int32_t w, int32_t h) {
    if (data) {
        free(data);
        data = nullptr;
    }
    if (w > 0 && h > 0) {
        // TODO: demo image path
        cv::Mat img = cv::imread("/data/user/0/com.scliang.tars/app_files/cpp.png");
        cv::cvtColor(img, img, cv::COLOR_BGRA2RGBA);
        data = (uint32_t *) malloc(sizeof(uint32_t) * img.rows * img.cols);
        memcpy(data, img.data, sizeof(uint32_t) * img.rows * img.cols);
        log_d("create test data. %d,%d", img.cols, img.rows);
    }
}
#endif

media::common::common(std::string &cascade, std::string &mnn)
:cascade(cascade),mnn(mnn),renderer(nullptr)
#if LOG_ABLE
,test_frame(nullptr)
#endif
{
    log_d("created. cascade:%s, mnn:%s", cascade.c_str(), mnn.c_str());
}

media::common::~common() {
    delete renderer;
#if LOG_ABLE
    delete test_frame;
#endif
    log_d("release.");
}

void media::common::renderer_init() {
    delete renderer;
    renderer = new image_renderer();
}

void media::common::renderer_release() {
    delete renderer;
    renderer = nullptr;
}

void media::common::renderer_surface_created() {
    if (renderer) {
        renderer->surface_created();
    }
}

void media::common::renderer_surface_destroyed() {
    if (renderer) {
        renderer->surface_destroyed();
    }
}

void media::common::renderer_surface_changed(int32_t w, int32_t h) {
    if (renderer) {
        renderer->surface_changed(w, h);
    }

#if LOG_ABLE
    // TODO: setup test frame
    if (test_frame == nullptr) {
        test_frame = new frame();
        test_frame->setup_test_data(w, h);
    }
#endif
}

void media::common::renderer_draw_frame() {

#if LOG_ABLE
    // TODO: draw test frame
    if (test_frame && renderer) {
        test_frame->draw_to_renderer(renderer);
    }
#endif
}
