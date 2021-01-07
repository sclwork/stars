//
// Created by scliang on 1/7/21.
//

#include "log.h"
#include "camera_paint.h"
#include "image_renderer.h"

#define log_d(...)  LOG_D("Media-Native:image_renderer", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:image_renderer", __VA_ARGS__)

namespace media {
} //namespace media

media::image_renderer::image_renderer():width(0),height(0),paint(nullptr) {
    log_d("created.");
}

media::image_renderer::~image_renderer() {
    delete paint;
    log_d("release.");
}

void media::image_renderer::surface_created() {
    delete paint;
    paint = new camera_paint();
}

void media::image_renderer::surface_destroyed() {
    delete paint;
    paint = nullptr;
}

void media::image_renderer::surface_changed(int32_t w, int32_t h) {
    width = w;
    height = h;
    if (paint) {
        paint->set_canvas_size(w, h);
    }
}

void media::image_renderer::draw_frame(int32_t w, int32_t h, uint32_t *data) {
    if (paint == nullptr || data == nullptr) {
        return;
    }

    paint->draw(w, h, data);
}
