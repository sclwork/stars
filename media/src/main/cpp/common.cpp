//
// Created by scliang on 1/6/21.
//

#include <memory>
#include "loop/loop.h"
#include "common.h"

#define log_d(...)  LOG_D("Media-Native:common", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:common", __VA_ARGS__)

namespace media {
    static common *gcom = nullptr;
} //namespace media

media::common::common(std::string &&cas_path, std::string &&mnn_path)
:mnn_path(mnn_path), renderer(nullptr), vid_rec(nullptr) {
    gcom = this;
    log_d("cas_path:%s.", cas_path.c_str());
    log_d("mnn_path:%s.", mnn_path.c_str());
    log_d("created.");
}

media::common::~common() {
    renderer.reset();
    vid_rec.reset();
    gcom = nullptr;
    log_d("release.");
}

/*
 * run in renderer thread.
 */
void media::common::renderer_init() {
    renderer = std::make_shared<image_renderer>();
    vid_rec = std::make_shared<video_recorder>(mnn_path);
}

/*
 * run in renderer thread.
 */
void media::common::renderer_release() {
    renderer.reset();
    vid_rec.reset();
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
    if (vid_rec != nullptr) {
        vid_rec->stop_preview();
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_changed(int32_t w, int32_t h) {
    if (renderer != nullptr) {
        renderer->surface_changed(w, h);
    }
    if (vid_rec != nullptr) {
        vid_rec->start_preview([](std::shared_ptr<media::image_frame> &&frm) {
            if (gcom != nullptr) gcom->renderer_updt_frame(*(frm.get()));
        }, w, h);
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_draw_frame() {
    if (renderer != nullptr) {
        renderer->draw_frame();
    }
}

/*
 * run in caller thread.
 * copy from frm to renderer.
 */
void media::common::renderer_updt_frame(const media::image_frame &frm) {
    if (renderer != nullptr) {
        renderer->updt_frame(std::make_shared<media::image_frame>(frm));
    }
}
