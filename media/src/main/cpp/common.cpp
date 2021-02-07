//
// Created by scliang on 1/6/21.
//

#include <memory>
#include "loop/loop.h"
#include "image/collect/image_recorder.h"
#include "image/play/image_renderer.h"
#include "common.h"

#define log_d(...)  LOG_D("Media-Native:common", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:common", __VA_ARGS__)

namespace media {
} //namespace media

media::common::common(std::string &&cas_path, std::string &&mnn_path)
:renderer(nullptr), shw_frame(nullptr) {
    log_d("cas_path:%s.", cas_path.c_str());
    log_d("mnn_path:%s.", mnn_path.c_str());
    log_d("created.");
}

media::common::~common() {
    delete renderer;
    shw_frame.reset();
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
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_created() {
    if (renderer != nullptr) {
        renderer->surface_created();
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_destroyed() {
    if (renderer != nullptr) {
        renderer->surface_destroyed();
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_changed(int32_t w, int32_t h) {
    if (shw_frame == nullptr || !shw_frame->available() || !shw_frame->same_size(w, h)) {
        shw_frame.reset(image_frame::make_default(w, h));
    }
    if (renderer != nullptr) {
        renderer->surface_changed(w, h);
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_draw_frame() {
    if (renderer != nullptr && shw_frame != nullptr) {
        renderer->draw_frame(shw_frame);
    }
}
