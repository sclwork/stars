//
// Created by Scliang on 2/7/21.
//

#include <thread>
#include "log.h"
#include "video_recorder.h"
#include "proc/mnn.h"

#define log_d(...)  LOG_D("Media-Native:video_recorder", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:video_recorder", __VA_ARGS__)

namespace media {

class sparams {
public:
    sparams(int32_t w, int32_t h, int32_t camera,
            std::string &mnn_path,
            std::shared_ptr<std::atomic_bool> &runnable,
            void (*callback)(std::shared_ptr<image_frame>&&))
           :w(w), h(h), camera(camera), mnn_path(mnn_path),
            runnable(runnable), callback(callback), image(nullptr), mnn(nullptr) {
        log_d("sparams[%d,%d,%d] created.", this->w, this->h, this->camera);
    }
    ~sparams() { log_d("sparams release."); }

public:
    int32_t camera_id() const { return camera; }

    bool equal_running(int32_t w_, int32_t h_, int32_t camera_) {
        return w == w_ && h == h_ && camera == camera_ && runnable != nullptr && *(runnable);
    }

    bool running() const { return runnable != nullptr && *runnable; }

    void activate() {
        mnn = new media::mnn(mnn_path, 4);
        image = new image_recorder();
        image->update_size(w, h);
        image->select_camera(camera);
    }

    void deactivate() {
        delete image;
        delete mnn;
    }

    void unrunning() {
        if (runnable != nullptr) {
            *(runnable) = false;
        }
    }

    void run() {
        auto frame = image->collect_frame();
        mnn->detect_faces(frame, faces);
        mnn->flag_faces(frame, faces);
        callback(std::forward<std::shared_ptr<media::image_frame>>(frame));
    }

private:
    int32_t w, h, camera;
    std::string mnn_path;
    std::shared_ptr<std::atomic_bool> runnable;
    void (*callback)(std::shared_ptr<media::image_frame>&&);
    std::vector<cv::Rect> faces;
    image_recorder *image;
    mnn *mnn;
};

static std::vector<std::shared_ptr<sparams>> st_sps;

static void img_collect_run(sparams *sp) {
    sp->activate();
    log_d("camera %d collect started ...", sp->camera_id());
    while (sp->running()) {
        sp->run();
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
    }
    sp->deactivate();
    log_d("camera %d collect stoped ...", sp->camera_id());
    delete sp;
}

} //namespace media

media::video_recorder::video_recorder(std::string &mnn_path):mnn_path(mnn_path) {
    log_d("created.");
}

media::video_recorder::~video_recorder() {
    stop_preview();
    log_d("release.");
}

void media::video_recorder::start_preview(void (*callback)(std::shared_ptr<image_frame>&&),
                                          int32_t w, int32_t h,
                                          int32_t camera) {
    for (auto &sp : st_sps) {
        if (sp != nullptr && sp->equal_running(w, h, camera)) {
            return;
        }
        if (sp != nullptr) {
            sp->unrunning();
        }
    }
    auto runnable = std::make_shared<std::atomic_bool>(true);
    auto *ctx = new sparams(w, h, camera, mnn_path, runnable, callback);
    st_sps.push_back(std::shared_ptr<sparams>(ctx, [](void*){})); // delete sp in img_collect_run
    std::thread collect_t(img_collect_run, ctx);
    collect_t.detach();
}

void media::video_recorder::stop_preview() {
    for (auto &sp : st_sps) {
        if (sp != nullptr) {
            sp->unrunning();
        }
    }
    st_sps.clear();
}
