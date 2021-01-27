//
// Created by scliang on 1/6/21.
//

#include "log.h"
#include "image_recorder.h"

#define log_d(...)  LOG_D("Media-Native:image_recorder", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:image_recorder", __VA_ARGS__)

namespace media {
} //namespace media

media::image_recorder::image_recorder()
:fps(0), recording(false), previewing(false), name(),
frame(nullptr), cams(), run_cam(nullptr), collect_mutex() {
    log_d("created.");
    camera::enumerate(cams);
//    log_d("--------------------");
//    for (const auto& c : cams) {
//        log_d("found cam %s", c->get_id().c_str());
//    }
//    log_d("--------------------");
}

media::image_recorder::~image_recorder() {
    run_cam = nullptr;
    previewing = false;
    log_d("release.");
}

void media::image_recorder::set_name(std::string &&n) {
    name = n;
}

std::string media::image_recorder::get_name() const {
    return std::string(name);
}

int32_t media::image_recorder::camera_count() const {
    return cams.size();
}

void media::image_recorder::destroy() {
    std::lock_guard<std::mutex> lock(collect_mutex);
    if (run_cam) { run_cam->close(); }
    run_cam = nullptr;
    previewing = false;
    log_d("destroy.");
}

void media::image_recorder::set_recording(bool ing) {
    recording = ing;
}

bool media::image_recorder::is_recording() const {
    return recording;
}

bool media::image_recorder::is_previewing() const {
    return previewing;
}

bool media::image_recorder::select_camera(int camera) {
    std::lock_guard<std::mutex> lock(collect_mutex);
    if (camera < 0 || camera >= cams.size()) {
        previewing = false;
        return false;
    }

    if (run_cam) {
        run_cam->close();
        run_cam = nullptr;
    }

    if (frame == nullptr) {
        previewing = false;
        return false;
    }

    int32_t w, h;
    frame->get(&w, &h);
    previewing = cams[camera]->preview(w, h, &fps);
    run_cam = cams[camera];
    return previewing;
}

void media::image_recorder::update_size(int32_t w, int32_t h) {
    std::lock_guard<std::mutex> lock(collect_mutex);
    previewing = false;
    // check reset cache
    if (frame == nullptr || !frame->same_size(w, h)) {
        frame = std::make_shared<image_frame>(w, h);
    }
    // restart cam
    if (run_cam) {
        run_cam->close();
        previewing = run_cam->preview(w, h, &fps);
    }
}

int32_t media::image_recorder::get_fps() const {
    return fps;
}

uint32_t media::image_recorder::get_width() const {
    if (frame == nullptr) {
        return 0;
    }
    int32_t width;
    frame->get(&width, nullptr, nullptr);
    return width;
}

uint32_t media::image_recorder::get_height() const {
    if (frame == nullptr) {
        return 0;
    }
    int32_t height;
    frame->get(nullptr, &height, nullptr);
    return height;
}

std::shared_ptr<media::image_frame> media::image_recorder::collect_frame() {
    std::lock_guard<std::mutex> lock(collect_mutex);
    if (frame == nullptr || !frame->available() || run_cam == nullptr) {
        return frame;
    }

    if (!is_previewing()) {
        return frame;
    }

    run_cam->get_latest_image(frame);
    return frame;
}
