//
// Created by Scliang on 2/7/21.
//

#include <thread>
#include "log.h"
#include "video_recorder.h"

#define log_d(...)  LOG_D("Media-Native:video_recorder", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:video_recorder", __VA_ARGS__)

media::video_recorder::video_recorder() {
    log_d("created.");
}

media::video_recorder::~video_recorder() {
    stop_preview();
    log_d("release.");
}

namespace media {

typedef struct _sparams {
    int32_t w, h, camera;
    std::shared_ptr<std::atomic_bool> runnable;
    void (*callback)(std::shared_ptr<media::image_frame>&&);
    _sparams(int32_t w, int32_t h, int32_t camera,
             std::shared_ptr<std::atomic_bool> &runnable,
             void (*callback)(std::shared_ptr<image_frame>&&))
        :w(w), h(h), camera(camera), runnable(runnable), callback(callback) {}
} _sparams;

static std::vector<std::shared_ptr<std::atomic_bool>> st_runnables;

static void img_collect_run(_sparams *sp) {
    int32_t w = sp->w;
    int32_t h = sp->h;
    int32_t camera = sp->camera;
    auto callback = sp->callback;
    std::shared_ptr<std::atomic_bool> runnable(sp->runnable);
    delete sp;
    auto *img = new image_recorder();
    img->update_size(w, h);
    img->select_camera(camera);
    log_d("camera %d collect started ...", camera);
    while (runnable != nullptr && *runnable) {
        auto frame = img->collect_frame();
        callback(std::forward<std::shared_ptr<media::image_frame>>(frame));
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
    }
    delete img;
    log_d("camera %d collect stoped ...", camera);
    runnable.reset();
}

} //namespace media

void media::video_recorder::start_preview(void (*callback)(std::shared_ptr<image_frame>&&),
                                          int32_t w, int32_t h,
                                          int32_t camera) {
    for (auto &st_runnable : st_runnables) {
        if (st_runnable != nullptr) { *st_runnable = false; }
    }
    st_runnables.clear();
    auto runnable = std::make_shared<std::atomic_bool>(true);
    st_runnables.push_back(runnable);
    auto *ctx = new _sparams(w, h, camera, runnable, callback);
    std::thread collect_t(img_collect_run, ctx);
    collect_t.detach();
}

void media::video_recorder::stop_preview() {
    for (auto &st_runnable : st_runnables) {
        if (st_runnable != nullptr) { *st_runnable = false; }
    }
    st_runnables.clear();
}
