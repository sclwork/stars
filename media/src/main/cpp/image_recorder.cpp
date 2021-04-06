//
// Created by scliang on 1/6/21.
//

#include "jni_log.h"
#include "image_recorder.h"

#define log_d(...)  LOG_D("Media-Native:image_recorder", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:image_recorder", __VA_ARGS__)

namespace media {
} //namespace media

media::image_recorder::image_recorder()
:previewing(false), width(0), height(0), fps(0), cams(), run_cam(nullptr) {
    log_d("created.");
    camera::enumerate(cams);
//    log_d("--------------------");
//    for (const auto& c : cams) {
//        log_d("found cam %s", c->get_id().c_str());
//    }
//    log_d("--------------------");
}

media::image_recorder::~image_recorder() {
    if (run_cam != nullptr) {
        run_cam->close();
        run_cam = nullptr;
    }
    previewing = false;
    log_d("release.");
}

bool media::image_recorder::select_camera(int camera) {
    if (camera < 0 || camera >= cams.size()) {
        previewing = false;
        return false;
    }

    if (run_cam != nullptr) {
        run_cam->close();
        run_cam = nullptr;
    }

    previewing = cams[camera]->preview(width, height, &fps);
    run_cam = cams[camera];
    return previewing;
}

void media::image_recorder::update_size(int32_t w, int32_t h) {
    previewing = false;
    // reset width/height/cache
    width = w;
    height = h;
    // restart cam
    if (run_cam != nullptr) {
        run_cam->close();
        previewing = run_cam->preview(w, h, &fps);
    }
}

void media::image_recorder::collect_frame(media::image_frame &of) {
    if (run_cam == nullptr) {
        return;
    }

    if (!is_previewing()) {
        return;
    }

    // check/setup frame width/height/cache
    if (!of.available() || !of.same_size(width, height)) {
        of.update_size(width, height);
    }

    run_cam->get_latest_image(of);
}
