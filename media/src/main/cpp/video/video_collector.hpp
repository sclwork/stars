//
// Created by Scliang on 3/23/21.
//

#ifndef STARS_VIDEO_COLLECTOR_HPP
#define STARS_VIDEO_COLLECTOR_HPP

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

#define log_d_(...)  LOG_D("Media-Native:video_recorder", __VA_ARGS__)
#define log_e_(...)  LOG_E("Media-Native:video_recorder", __VA_ARGS__)

namespace media {

class video_collector {
public:
    video_collector(bool use_mnn, bool use_opencv,
                    int32_t w, int32_t h, int32_t camera,
                    std::string &mnn_path,
                    std::shared_ptr<std::atomic_bool> &runnable,
                    std::shared_ptr<std::atomic_bool> &recording,
                    void (*callback)(std::shared_ptr<image_frame>&&),
                    std::shared_ptr<moodycamel::ConcurrentQueue<frame>> &&fQ)
     :ms(0), tv(), use_mnn(use_mnn), use_opencv(use_opencv),
      w(w), h(h), camera(camera), fps_ms(0), mnn_path(mnn_path),
      runnable(runnable), recording(recording), callback(callback), _mux(),
      image(nullptr), audio(nullptr), mnn(nullptr), opencv(nullptr),
      img_args(), aud_args(), aud_frame(nullptr), frameQ(fQ) {
        log_d_("video_collector[%d,%d,%d] created.", this->w, this->h, this->camera);
    }

    ~video_collector() {
        log_d_("video_collector release.");
    }

public:
    int32_t camera_id() const { return camera; }

    bool equal_running(int32_t w_, int32_t h_, int32_t camera_) {
        std::lock_guard<std::mutex> lg(_mux);
        return w == w_ && h == h_ && camera == camera_ && runnable != nullptr && *(runnable);
    }

    bool running() const {
        std::lock_guard<std::mutex> lg(_mux);
        return runnable != nullptr && *runnable;
    }

    void activate() {
        if (use_mnn) {
            mnn = new media::mnn(mnn_path, 1);
        }
        if (use_opencv) {
            opencv = new media::opencv();
        }
        image = new image_recorder();
        image->update_size(w, h);
        image->select_camera(camera);
        int32_t fps = image->get_fps();
        if (fps > 0) {
            fps_ms = (int32_t) (1000.0f / fps);
        }
        audio = new audio_recorder();
        img_args.width = w;
        img_args.height = h;
        img_args.fps = fps;
        img_args.channels = image->get_channels();
        aud_args.sample_rate = audio->get_sample_rate();
        aud_args.frame_size = audio->get_frame_size();
        aud_args.channels = audio->get_channels();
    }

    void deactivate() {
        delete image;
        delete audio;
        delete mnn;
        delete opencv;
    }

    void unrunning() {
        std::lock_guard<std::mutex> lg(_mux);
        if (runnable != nullptr) {
            *(runnable) = false;
        }
    }

    static void audio_frame_callback(void *ctx) {
        if (ctx == nullptr) {
            return;
        }
        auto *cps = (video_collector *)ctx;
        if (cps->recording != nullptr && *cps->recording && cps->frameQ != nullptr && cps->audio != nullptr) {
            auto aud_frame = cps->audio->collect_frame(nullptr);
            auto frame = media::frame(nullptr, std::forward<std::shared_ptr<audio_frame>>(aud_frame));
            cps->frameQ->enqueue(frame);
        }
    }

    void set_record(bool recing) {
        std::lock_guard<std::mutex> lg(_mux);
        if (recording != nullptr) {
            *(recording) = recing;
        }
        if (audio != nullptr) {
            if (recing) { audio->start_record(audio_frame_callback, this); }
            else { audio->stop_record(); }
        }
    }

    void copy_args(image_args &img, audio_args &aud) const {
        img.channels = img_args.channels;
        img.frame_size = img_args.frame_size;
        img.fps = img_args.fps;
        img.width = img_args.width;
        img.height = img_args.height;
        aud.frame_size = aud_args.frame_size;
        aud.channels = aud_args.channels;
        aud.sample_rate = aud_args.sample_rate;
    }

    void run() {
        gettimeofday(&tv, nullptr);
        ms = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        auto img_frame = image->collect_frame();
        if (use_opencv) {
            opencv::grey_frame(img_frame);
        }
        if (use_mnn) {
            mnn->detect_faces(img_frame, faces);
            mnn->flag_faces(img_frame, faces);
        }
        if (recording != nullptr && *recording && frameQ != nullptr) {
            auto frame = media::frame(std::forward<std::shared_ptr<image_frame>>(img_frame), nullptr);
            frameQ->enqueue(frame);
        }
        callback(std::forward<std::shared_ptr<media::image_frame>>(img_frame));
        gettimeofday(&tv, nullptr);
        ms = tv.tv_sec * 1000 + tv.tv_usec / 1000 - ms;
        ms = fps_ms - ms;
        if (ms > 0) {
//            log_d("need wait time: %ldms.", ms);
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
    }

private:
    long ms;
    struct timeval tv;
    bool use_mnn, use_opencv;
    int32_t w, h, camera, fps_ms;
    std::string mnn_path;
    std::shared_ptr<std::atomic_bool> runnable;
    std::shared_ptr<std::atomic_bool> recording;
    void (*callback)(std::shared_ptr<media::image_frame>&&);
    std::vector<cv::Rect> faces;
    mutable std::mutex _mux;
    image_recorder *image;
    audio_recorder *audio;
    mnn *mnn;
    opencv *opencv;
    image_args img_args;
    audio_args aud_args;
    std::shared_ptr<media::audio_frame> aud_frame;
    std::shared_ptr<moodycamel::ConcurrentQueue<frame>> frameQ;
};

} //namespace media

#endif //STARS_VIDEO_COLLECTOR_HPP
