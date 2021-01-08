//
// Created by scliang on 1/6/21.
//

#include "log.h"
#include "image_recorder.h"

#define log_d(...)  LOG_D("Media-Native:image_recorder", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:image_recorder", __VA_ARGS__)

namespace media {

image_recorder::image_recorder(int32_t width, int32_t height)
:width(width), height(height), cams(), run_cam(nullptr) {
    log_d("created.");
    camera::enumerate(cams);
    log_d("--------------------");
    for (const auto& c : cams) {
        log_d("found cam %s", c->get_id().c_str());
    }
    log_d("--------------------");
    run_cam = cams[0];
    run_cam->preview(width, height);
}

image_recorder::~image_recorder() {
    run_cam = nullptr;
    log_d("release.");
}

void image_recorder::update_size(int32_t w, int32_t h) {
    if (w == width && h == height) {
        return;
    }

    width = w;
    height = h;

    // restart cam
    if (run_cam) {
        run_cam->close();
        run_cam->preview(width, height);
    }
}

} //namespace media
