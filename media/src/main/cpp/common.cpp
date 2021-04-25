//
// Created by scliang on 1/6/21.
//

#include <memory>
#include "loop.h"
#include "common.h"

#define log_d(...)  LOG_D("Media-Native:common", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:common", __VA_ARGS__)

namespace media {
    static common *gcom = nullptr;
} //namespace media

media::common::common(const std::string &file_root,
                      const std::string &cas_path,
                      const std::string &mnn_path,
                      void (*on_request_render)(int32_t))
:file_root(file_root), mnn_path(mnn_path), renderer(nullptr), vid_rec(nullptr),
eiQ(), eaQ(), on_request_render_callback(on_request_render), vid_size() {
    gcom = this;
    log_d("file_root:%s.", file_root.c_str());
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
    renderer = std::make_shared<image_renderer>(file_root, eiQ, eaQ, [](){
        return gcom != nullptr && gcom->video_recording();
    });
    vid_rec = std::make_shared<video_recorder>(mnn_path, eiQ, eaQ);
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
    vid_size[0] = w / 2;
    vid_size[1] = h / 2;
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
void media::common::renderer_updt_frame(image_frame &&frm) {
    if (renderer != nullptr) {
        renderer->updt_frame(std::forward<image_frame>(frm));
    }
    if (on_request_render_callback != nullptr) {
        on_request_render_callback(0);
    }
}

/*
 * run in media main loop thread.
 */
void media::common::video_record_start(std::string &&name) {
    if (vid_rec != nullptr) {
        vid_rec->start_record(std::forward<std::string>(file_root),
                              std::forward<std::string>(name));
    }
}

/*
 * run in media main loop thread.
 */
void media::common::video_record_stop() {
    if (vid_rec != nullptr) {
        vid_rec->stop_record();
    }
}

/*
 * run in caller thread.
 */
bool media::common::video_recording() {
    if (vid_rec != nullptr) {
        return vid_rec->recording();
    } else {
        return false;
    }
}

/*
 * run in media main loop thread.
 */
void media::common::camera_select(int32_t cam) {
    if (vid_rec != nullptr) {
        vid_rec->start_preview([](image_frame &&frm) {
            if (gcom != nullptr) gcom->renderer_updt_frame(std::forward<image_frame>(frm));
        }, vid_size[0], vid_size[1], cam);
    }
}
