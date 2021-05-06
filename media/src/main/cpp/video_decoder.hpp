//
// Created by Scliang on 3/23/21.
//

#ifndef STARS_VIDEO_DECODER_HPP
#define STARS_VIDEO_DECODER_HPP

#include <ctime>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "jni_log.h"
#include "video_recorder.h"
#include "image_recorder.h"
#include "audio_recorder.h"
#include "utils.h"
#include "mnn.h"
#include "opencv.h"
#include "h264_encoder.h"
#include "loudnorm.h"
#include "webrtc_ns.h"

#define log_d(...)  LOG_D("Media-Native:video_decoder", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:video_decoder", __VA_ARGS__)

namespace media {

static void get_file_not_exists_tip_frame(int32_t w, int32_t h, uint32_t *cache) {
    if (cache != nullptr) {
        cv::Mat img = cv::imread("/data/user/0/com.scliang.tars/app_files/ic_vid_file_not_exists.png");
        cv::cvtColor(img, img, cv::COLOR_BGRA2RGBA);
        int32_t wof = (w - img.cols) / 2;
        int32_t hof = (h - img.rows) / 2;
        for (int32_t i = 0; i < img.rows; i++) {
            for (int32_t j = 0; j < img.cols; j++) {
                cache[(i+hof)*w+j+wof] =
                        (((int32_t)img.data[(i*img.cols+j)*4+3])<<24) +
                        (((int32_t)img.data[(i*img.cols+j)*4+2])<<16) +
                        (((int32_t)img.data[(i*img.cols+j)*4+1])<<8) +
                        (img.data[(i*img.cols+j)*4]);
            }
        }
    }
}

class video_decoder {
public:
    video_decoder(int32_t w, int32_t h, std::string &&n,
                  std::shared_ptr<std::atomic_bool> &runnable,
                  void (*callback)(image_frame&&))
    :ms(0), tv(), _mux(), width(w), height(h), fps_ms((int32_t)(1000.0f/30.0f)),
    name(n), runnable(runnable), renderer_callback(callback) {
        log_d("created.");
    }

    ~video_decoder() {
        log_d("release.");
    }

public:
    bool running() const {
        std::lock_guard<std::mutex> lg(_mux);
        return runnable != nullptr && *runnable;
    }

    void unrunning() {
        std::lock_guard<std::mutex> lg(_mux);
        if (runnable != nullptr) {
            *(runnable) = false;
        }
    }

    void run() {
        bool exists = file_exists(name);
//        log_d("start video play:[%d,%d;%d] %s.", width, height, exists, name.c_str());
        if (!exists) {
            gettimeofday(&tv, nullptr);
            ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;
            image_frame f(width, height);
            uint32_t *cache = nullptr;
            f.get(nullptr, nullptr, &cache);
            get_file_not_exists_tip_frame(width, height, cache);
            if (f.available()) {
                if (renderer_callback != nullptr) renderer_callback(std::forward<image_frame>(f));
            }
            gettimeofday(&tv, nullptr);
            ms = tv.tv_sec * 1000 + tv.tv_usec / 1000 - ms;
            ms = fps_ms - ms;
            if (ms > 0) {
//                log_d("need wait time: %ldms.", ms);
                std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            }
            return;
        }
    }

private:
    long ms;
    struct timeval tv;
    mutable std::mutex _mux;
    int32_t width, height, fps_ms;
    std::string name;
    std::shared_ptr<std::atomic_bool> runnable;
    void (*renderer_callback)(image_frame&&);
};

} //namespace media

#endif //STARS_VIDEO_DECODER_HPP
