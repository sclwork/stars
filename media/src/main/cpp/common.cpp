//
// Created by scliang on 1/6/21.
//

#include "common.h"
#include "video/collect/image_recorder.h"
#include "video/play/image_renderer.h"

#define log_d(...)  LOG_D("Media-Native:common", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:common", __VA_ARGS__)

namespace media {
} //namespace media

void media::frame::draw_to_renderer(renderer *renderer) {
    if (data && renderer) {
        renderer->draw_frame(width, height, data);
    }
}

#if DRAW_TEST_FRAME
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
        data = (uint32_t *) malloc(sizeof(uint32_t) * w * h);
        width = w; height = h;
        int32_t wof = (width - img.cols) / 2;
        int32_t hof = (height - img.rows) / 2;
        for (int32_t i = 0; i < img.rows; i++) {
            for (int32_t j = 0; j < img.cols; j++) {
                data[(i+hof)*width+j+wof] =
                        (((int32_t)img.data[(i*img.cols+j)*4+3])<<24) +
                        (((int32_t)img.data[(i*img.cols+j)*4+2])<<16) +
                        (((int32_t)img.data[(i*img.cols+j)*4+1])<<8) +
                        (img.data[(i*img.cols+j)*4]);
            }
        }
        log_d("create test data. %d,%d", width, height);
    }
}
#endif

media::common::common(std::string &cascade, std::string &mnn)
:cascade(cascade),mnn(mnn),recorder(nullptr),renderer(nullptr)
#if DRAW_TEST_FRAME
,test_frame(nullptr)
#endif
{
    log_d("created. cascade:%s, mnn:%s", cascade.c_str(), mnn.c_str());
}

media::common::~common() {
    delete recorder;
    delete renderer;
#if DRAW_TEST_FRAME
    delete test_frame;
#endif
    log_d("release.");
}

/*
 * run in renderer thread.
 */
void media::common::renderer_init() {
    delete renderer;
    renderer = new image_renderer();
}

/*
 * run in renderer thread.
 */
void media::common::renderer_release() {
    delete renderer;
    renderer = nullptr;
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_created() {
    int32_t w = 720, h = 1280;
    if (renderer) {
        renderer->get_size(&w, &h);
    }
    delete recorder;
    recorder = new image_recorder(w, h);
    if (renderer) {
        renderer->surface_created();
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_destroyed() {
    if (renderer) {
        renderer->surface_destroyed();
    }
    delete recorder;
    recorder = nullptr;
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_changed(int32_t w, int32_t h) {
    if (renderer) {
        renderer->surface_changed(w, h);
    }
    if (recorder) {
        recorder->update_size(w, h);
    }

#if DRAW_TEST_FRAME
    // TODO: setup test frame
    if (test_frame == nullptr) {
        test_frame = new frame();
        test_frame->setup_test_data(w, h);
    }
#endif
}

/*
 * run in renderer thread.
 */
void media::common::renderer_draw_frame() {

#if DRAW_TEST_FRAME
    // TODO: draw test frame
    if (test_frame && renderer) {
        test_frame->draw_to_renderer(renderer);
    }
#endif
}
