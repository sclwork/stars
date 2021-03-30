//
// Created by scliang on 1/7/21.
//

#include "jni_log.h"
#include "camera_paint.h"
#include "image_renderer.h"

#define log_d(...)  LOG_D("Media-Native:image_renderer", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:image_renderer", __VA_ARGS__)

namespace media {
} //namespace media

media::image_renderer::image_renderer()
:width(0), height(0), paint(nullptr), frame(nullptr), frameQ() {
    log_d("created.");
}

media::image_renderer::~image_renderer() {
    delete paint;
    frame.reset();
    log_d("release.");
}

void media::image_renderer::surface_created() {
    delete paint;
    paint = new camera_paint();
    frame.reset();
}

void media::image_renderer::surface_destroyed() {
    delete paint;
    paint = nullptr;
    frame.reset();
}

void media::image_renderer::surface_changed(int32_t w, int32_t h) {
    width = w;
    height = h;
    if (paint) {
        paint->set_canvas_size(w, h);
    }
    if (frame == nullptr || !frame->available() || !frame->same_size(w, h)) {
        updt_frame(std::shared_ptr<media::image_frame>(image_frame::make_default(w, h)));
    }
}

void media::image_renderer::updt_frame(const std::shared_ptr<media::image_frame> &&frm) {
    if (frm != nullptr && frm->available()) {
        frameQ.enqueue(frm);
    }
}

void media::image_renderer::draw_frame() {
    if (paint == nullptr) {
        return;
    }

    std::shared_ptr<media::image_frame> f;
    if (frameQ.try_dequeue(f)) {
        frame = f;
    }

    if (frame != nullptr) {
        auto nf = paint->draw(frame);
        if (nf) frame->run_op_callback(nf);
        delete nf;
    }
}
