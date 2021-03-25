//
// Created by Scliang on 2/7/21.
//

#include <ctime>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "jni_log.h"
#include "video_recorder.h"
#include "image/collect/image_recorder.h"
#include "audio/collect/audio_recorder.h"
#include "proc/proc.h"
#include "proc/mnn.h"
#include "proc/opencv.h"
#include "proc/ffmpeg_mp4.h"
#include "proc/ffmpeg_rtmp.h"
#include "proc/ffmpeg_loudnorm.h"
#include "proc/webrtc_ns.h"

#define log_d(...)  LOG_D("Media-Native:video_recorder", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:video_recorder", __VA_ARGS__)

namespace media {

static void img_collect_run(video_collector *cp) {
    cp->activate();
    log_d("camera %d collect started ...", cp->camera_id());
    while (cp->running()) cp->run();
    cp->deactivate();
    log_d("camera %d collect stoped ...", cp->camera_id());
    delete cp;
}

static void img_encode_run(video_encoder *ep) {
    log_d("encode started.");
    while (ep->running()) ep->run();
    log_d("encode stoped ...");
    delete ep;
}

} //namespace media

media::video_recorder::video_recorder(std::string &mnn_path)
:mnn_path(mnn_path), recing(false),
frameQ(std::make_shared<moodycamel::ConcurrentQueue<frame>>()),
collector(nullptr), encoder(nullptr) {
    log_d("created.");
}

media::video_recorder::~video_recorder() {
    stop_preview();
    stop_record();
    log_d("release.");
}

void media::video_recorder::start_preview(void (*callback)(std::shared_ptr<image_frame>&&),
                                          int32_t w, int32_t h,
                                          int32_t camera) {
    if (collector != nullptr && collector->equal_running(w, h, camera)) {
        return;
    }
    if (collector != nullptr) {
        collector->unrunning();
        collector = nullptr;
    }
    auto runnable = std::make_shared<std::atomic_bool>(true);
    auto recording = std::make_shared<std::atomic_bool>(false);
    auto *ctx = new video_collector(true, false, w, h, camera,
                                    mnn_path, runnable, recording, callback,
                                    std::forward<std::shared_ptr<moodycamel::ConcurrentQueue<frame>>>(frameQ)
    );
    collector = std::shared_ptr<video_collector>(ctx, [](void*){}); // delete cp in img_collect_run
    std::thread collect_t(img_collect_run, ctx);
    collect_t.detach();
}

void media::video_recorder::stop_preview() {
    if (collector != nullptr) {
        collector->unrunning();
        collector = nullptr;
    }
}

void media::video_recorder::start_record(std::string &&file_root, std::string &&name) {
    log_d("start video record[%s]-[%s].", file_root.c_str(), name.c_str());
    image_args img_args{};
    audio_args aud_args{};
    if (collector != nullptr && collector->running()) {
        collector->copy_args(img_args, aud_args);
    }
    if (collector != nullptr) {
        collector->set_record(true);
    }
    if (encoder != nullptr) {
        encoder->unrunning();
        encoder = nullptr;
    }
    auto runnable = std::make_shared<std::atomic_bool>(true);
    auto *ctx = new video_encoder(true, 0,
                                  std::forward<std::string>(file_root), std::forward<std::string>(name),
                                  std::forward<image_args>(img_args), std::forward<audio_args>(aud_args), runnable,
                                  std::forward<std::shared_ptr<moodycamel::ConcurrentQueue<frame>>>(frameQ)
    );
    encoder = std::shared_ptr<video_encoder>(ctx, [](void*){}); // delete ep in img_encode_run
    std::thread encode_t(img_encode_run, ctx);
    encode_t.detach();
    recing = true;
}

void media::video_recorder::stop_record() {
    log_d("stop video record.");
    if (collector != nullptr) {
        collector->set_record(false);
    }
    recing = false;
    if (encoder != nullptr) {
        encoder->unrunning();
        encoder = nullptr;
    }
}
