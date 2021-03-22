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
#include "proc/ffmpeg_mp4.h"
#include "proc/ffmpeg_rtmp.h"
#include "proc/ffmpeg_loudnorm.h"
#include "proc/webrtc_ns.h"

#define log_d(...)  LOG_D("Media-Native:video_recorder", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:video_recorder", __VA_ARGS__)

namespace media {

class collect_params {
public:
    collect_params(int32_t w, int32_t h, int32_t camera,
                   std::string &mnn_path,
                   std::shared_ptr<std::atomic_bool> &runnable,
                   std::shared_ptr<std::atomic_bool> &recording,
                   void (*callback)(std::shared_ptr<image_frame>&&),
#ifdef USE_CONCURRENT_QUEUE
                   std::shared_ptr<moodycamel::ConcurrentQueue<frame>> &&fQ
#else
                   std::shared_ptr<safe_queue<frame>> &&fQ
#endif
                  ):ms(0), tv(), w(w), h(h), camera(camera), fps_ms(0), mnn_path(mnn_path),
                    runnable(runnable), recording(recording), callback(callback), _mux(),
                    image(nullptr), audio(nullptr), mnn(nullptr),
                    img_args(), aud_args(), aud_frame(nullptr), frameQ(fQ) {
        log_d("collect params[%d,%d,%d] created.", this->w, this->h, this->camera);
    }
    ~collect_params() { log_d("collect params release."); }

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
        mnn = new media::mnn(mnn_path, 1);
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
        auto *cps = (collect_params *)ctx;
        if (cps->recording != nullptr && *cps->recording && cps->frameQ != nullptr && cps->audio != nullptr) {
            auto aud_frame = cps->audio->collect_frame(nullptr);
            auto frame = media::frame(nullptr, std::forward<std::shared_ptr<audio_frame>>(aud_frame));
#ifdef USE_CONCURRENT_QUEUE
            cps->frameQ->enqueue(frame);
#else
            cps->frameQ->push(frame);
#endif
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
        mnn->detect_faces(img_frame, faces);
        mnn->flag_faces(img_frame, faces);
        if (recording != nullptr && *recording && frameQ != nullptr) {
            auto frame = media::frame(std::forward<std::shared_ptr<image_frame>>(img_frame), nullptr);
#ifdef USE_CONCURRENT_QUEUE
            frameQ->enqueue(frame);
#else
            frameQ->push(frame);
#endif
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
    image_args img_args;
    audio_args aud_args;
    std::shared_ptr<media::audio_frame> aud_frame;
#ifdef USE_CONCURRENT_QUEUE
    std::shared_ptr<moodycamel::ConcurrentQueue<frame>> frameQ;
#else
    std::shared_ptr<safe_queue<frame>> frameQ;
#endif
};

class encode_params {
public:
    encode_params(
        bool use_loudnorm,
        int32_t ns_mode,
        std::string &&file_root,
        std::string &&name,
        image_args &&img_args, audio_args &&aud_args,
        std::shared_ptr<std::atomic_bool> &runnable,
#ifdef USE_CONCURRENT_QUEUE
        std::shared_ptr<moodycamel::ConcurrentQueue<frame>> &&fQ
#else
        std::shared_ptr<safe_queue<frame>> &&fQ
#endif
    ):_mux(), runnable(runnable),
    loudnorm(use_loudnorm?std::make_shared<ffmpeg_loudnorm>("I=-16:tp=-1.5:LRA=11",
            std::forward<audio_args>(aud_args)):nullptr),
    ns(ns_mode>=0?std::make_shared<webrtc_ns>(ns_mode, aud_args.sample_rate):nullptr),
    mp4(endWith(name,".mp4")?std::make_shared<ffmpeg_mp4>(
            std::forward<std::string>(name),
            std::forward<image_args>(img_args), std::forward<audio_args>(aud_args)):nullptr),
    rtmp(startWith(name,"rtmp")?std::make_shared<ffmpeg_rtmp>(
            std::forward<std::string>(file_root), std::forward<std::string>(name),
            std::forward<image_args>(img_args), std::forward<audio_args>(aud_args)):nullptr),
    frameQ(fQ) {
        if (loudnorm != nullptr) loudnorm->init();
        if (ns != nullptr) ns->init();
        if (mp4 != nullptr) mp4->init();
        if (rtmp != nullptr) rtmp->init();
        log_d("encode params created.");
    }
    ~encode_params() {
        if (mp4 != nullptr) mp4->complete();
        if (rtmp != nullptr) rtmp->complete();
        if (ns != nullptr) ns->complete();
        if (loudnorm != nullptr) loudnorm->complete();
        log_d("encode params release.");
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
        if (frameQ != nullptr) {
            check_frameQ();
            check_loudnorm();
            check_ns();
        }
    }

private:
    void check_frameQ() {
        frame frm;
        if
#ifdef USE_CONCURRENT_QUEUE
        (frameQ->try_dequeue(frm))
#else
        (frameQ->try_pop(frm))
#endif
        {
            if (loudnorm != nullptr) {
                loudnorm->encode_frame(frm.audio);
                if (mp4 != nullptr) {
                    mp4->encode_image_frame(frm.image);
                }
                if (rtmp != nullptr) {
                    rtmp->encode_image_frame(frm.image);
                }
            } else {
                if (ns != nullptr) {
                    ns->encode_frame(frm.audio);
                    if (mp4 != nullptr) {
                        mp4->encode_image_frame(frm.image);
                    }
                    if (rtmp != nullptr) {
                        rtmp->encode_image_frame(frm.image);
                    }
                } else {
                    if (mp4 != nullptr) {
                        mp4->encode_image_frame(frm.image);
                        mp4->encode_audio_frame(frm.audio);
                    }
                    if (rtmp != nullptr) {
                        rtmp->encode_image_frame(frm.image);
                        rtmp->encode_audio_frame(frm.audio);
                    }
                }
            }
        }
    }

    void check_loudnorm() {
        if (loudnorm != nullptr) {
            std::shared_ptr<media::audio_frame> audio = loudnorm->get_encoded_frame();
            if (audio != nullptr) {
                if (ns != nullptr) {
                    ns->encode_frame(audio);
                } else {
                    if (mp4 != nullptr) {
                        mp4->encode_audio_frame(audio);
                    }
                    if (rtmp != nullptr) {
                        rtmp->encode_audio_frame(audio);
                    }
                }
            }
        }
    }

    void check_ns() {
        if (ns != nullptr) {
            std::shared_ptr<media::audio_frame> audio = ns->get_encoded_frame();
            if (audio != nullptr) {
                if (mp4 != nullptr) {
                    mp4->encode_audio_frame(audio);
                }
                if (rtmp != nullptr) {
                    rtmp->encode_audio_frame(audio);
                }
            }
        }
    }

private:
    mutable std::mutex _mux;
    std::shared_ptr<std::atomic_bool> runnable;
    std::shared_ptr<ffmpeg_loudnorm> loudnorm;
    std::shared_ptr<webrtc_ns> ns;
    std::shared_ptr<ffmpeg_mp4> mp4;
    std::shared_ptr<ffmpeg_rtmp> rtmp;
#ifdef USE_CONCURRENT_QUEUE
    std::shared_ptr<moodycamel::ConcurrentQueue<frame>> frameQ;
#else
    std::shared_ptr<safe_queue<frame>> frameQ;
#endif
};

static std::vector<std::shared_ptr<collect_params>> st_cps;
static std::vector<std::shared_ptr<encode_params>>  st_eps;

static void img_collect_run(collect_params *cp) {
    cp->activate();
    log_d("camera %d collect started ...", cp->camera_id());
    while (cp->running()) cp->run();
    cp->deactivate();
    log_d("camera %d collect stoped ...", cp->camera_id());
    delete cp;
}

static void img_encode_run(encode_params *ep) {
    log_d("encode started.");
    while (ep->running()) ep->run();
    log_d("encode stoped ...");
    delete ep;
}

} //namespace media

media::video_recorder::video_recorder(std::string &mnn_path)
:mnn_path(mnn_path), recing(false),
#ifdef USE_CONCURRENT_QUEUE
frameQ(std::make_shared<moodycamel::ConcurrentQueue<frame>>())
#else
frameQ(std::make_shared<safe_queue<frame>>())
#endif
{
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
    for (auto &cp : st_cps) {
        if (cp != nullptr && cp->equal_running(w, h, camera)) {
            return;
        }
        if (cp != nullptr) {
            cp->unrunning();
        }
    }
    auto runnable = std::make_shared<std::atomic_bool>(true);
    auto recording = std::make_shared<std::atomic_bool>(false);
    auto *ctx = new collect_params(w, h, camera, mnn_path, runnable, recording, callback,
#ifdef USE_CONCURRENT_QUEUE
        std::forward<std::shared_ptr<moodycamel::ConcurrentQueue<frame>>>(frameQ)
#else
        std::forward<std::shared_ptr<safe_queue<frame>>>(frameQ)
#endif
    );
    st_cps.push_back(std::shared_ptr<collect_params>(ctx, [](void*){})); // delete cp in img_collect_run
    std::thread collect_t(img_collect_run, ctx);
    collect_t.detach();
}

void media::video_recorder::stop_preview() {
    for (auto &cp : st_cps) {
        if (cp != nullptr) {
            cp->unrunning();
        }
    }
    st_cps.clear();
}

void media::video_recorder::start_record(std::string &&file_root, std::string &&name) {
    log_d("start video record[%s]-[%s].", file_root.c_str(), name.c_str());
    image_args img_args{};
    audio_args aud_args{};
    for (auto &cp : st_cps) {
        if (cp != nullptr && cp->running()) {
            cp->copy_args(img_args, aud_args);
            break;
        }
    }
    for (auto &cp : st_cps) {
        if (cp != nullptr) {
            cp->set_record(true);
        }
    }
    for (auto &ep : st_eps) {
        if (ep != nullptr) {
            ep->unrunning();
        }
    }
    auto runnable = std::make_shared<std::atomic_bool>(true);
    auto *ctx = new encode_params(true, 0,
            std::forward<std::string>(file_root), std::forward<std::string>(name),
            std::forward<image_args>(img_args), std::forward<audio_args>(aud_args), runnable,
#ifdef USE_CONCURRENT_QUEUE
            std::forward<std::shared_ptr<moodycamel::ConcurrentQueue<frame>>>(frameQ)
#else
            std::forward<std::shared_ptr<safe_queue<frame>>>(frameQ)
#endif
    );
    st_eps.push_back(std::shared_ptr<encode_params>(ctx, [](void*){})); // delete ep in img_encode_run
    std::thread encode_t(img_encode_run, ctx);
    encode_t.detach();
    recing = true;
}

void media::video_recorder::stop_record() {
    log_d("stop video record.");
    for (auto &cp : st_cps) {
        if (cp != nullptr) {
            cp->set_record(false);
        }
    }
    recing = false;
    for (auto &ep : st_eps) {
        if (ep != nullptr) {
            ep->unrunning();
        }
    }
    st_eps.clear();
}
