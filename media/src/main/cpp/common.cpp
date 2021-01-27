//
// Created by scliang on 1/6/21.
//

#include <memory>
#include "loop/loop.h"
#include "video/collect/image_recorder.h"
#include "video/play/image_renderer.h"
#include "common.h"

#define log_d(...)  LOG_D("Media-Native:common", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:common", __VA_ARGS__)

namespace media {
} //namespace media

void media::show_frame::set_frame(std::shared_ptr<media::image_frame> &&frm) {
    frame = frm;
}

std::shared_ptr<media::image_frame> media::show_frame::get_frame() const {
    return std::shared_ptr<media::image_frame>(frame);
}

std::mutex &media::show_frame::get_mutex() {
    return frame_mutex;
}

media::mnns::mnns(std::string &mnn_path):mnn_arr(), thrd_ids() {
    mnn_arr[0] = std::make_shared<media::mnn>(mnn_path, 1);
    mnn_arr[1] = std::make_shared<media::mnn>(mnn_path, 1);
    mnn_arr[2] = std::make_shared<media::mnn>(mnn_path, 1);
}

std::shared_ptr<media::mnn> media::mnns::get_mnn(std::__thread_id id) {
    std::__thread_id z;
    for (int32_t i = 0; i < 3; ++i) {
        if (z == thrd_ids[i]) {
            thrd_ids[i] = id;
            return mnn_arr[i];
        } else if (id == thrd_ids[i]) {
            return mnn_arr[i];
        }
    }
    return mnn_arr[0];
}

media::common::common(std::string &cas_path, std::string &mnn_path)
:cas_path(cas_path), renderer(nullptr),
img_recorder(nullptr), aud_recorder(nullptr), shw_frame(std::make_shared<show_frame>()),
#if LOG_ABLE && LOG_DRAW_TIME
draw_t(0),
#endif
#if IMPORT_TFLITE
tflite(std::make_shared<media::tflite>()),
#endif
mnns(std::make_shared<media::mnns>(mnn_path)), ffmpeg(std::make_shared<media::ffmpeg>()) {
    log_d("created.");
//    log_d("created. cascade:%s.", cas_path.c_str());
}

media::common::~common() {
    renderer.reset();
    img_recorder.reset();
    aud_recorder.reset();
    log_d("release.");
}

/*
 * run in renderer thread.
 */
void media::common::renderer_init() {
    renderer = std::make_shared<image_renderer>();
}

/*
 * run in renderer thread.
 */
void media::common::renderer_release() {
    renderer.reset();
}

/*
 * run in renderer thread.
 */
int32_t media::common::renderer_surface_created() {
    aud_recorder = std::make_shared<audio_recorder>();
    img_recorder = std::make_shared<image_recorder>();
    if (renderer != nullptr) {
        renderer->surface_created();
    }
    return img_recorder != nullptr ? img_recorder->camera_count() : 0;
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_destroyed() {
    if (renderer != nullptr) {
        renderer->surface_destroyed();
    }
    img_recorder->destroy();
    aud_recorder->destroy();
    img_recorder.reset();
    aud_recorder.reset();
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_changed(int32_t w, int32_t h) {
    if (renderer != nullptr) {
        renderer->surface_changed(w, h);
    }
    if (img_recorder != nullptr) {
        img_recorder->update_size(w, h);
    }
}

namespace media {

class record_collect_ctx {
public:
    explicit record_collect_ctx(std::shared_ptr<show_frame> &&sf,
                                std::shared_ptr<ffmpeg> &&ff,
                                std::shared_ptr<mnns> &&mns,
                                std::shared_ptr<image_recorder> &&img,
                                std::shared_ptr<audio_recorder> &&aud)
                                    :shw_frame(sf), ffmpeg(ff), mnns(mns), image(img), audio(aud) {}
    ~record_collect_ctx() = default;

public:
    void run() {
#if LOG_ABLE && LOG_DRAW_TIME
        long ens = 0;
        struct timespec st{0,0};
        clock_gettime(CLOCK_REALTIME, &st);
#endif
        if (image == nullptr || audio == nullptr) {
            return;
        }

//#if LOG_ABLE && LOG_DRAW_TIME
//    clock_gettime(CLOCK_REALTIME, &frame_args.t);
//    frame_args.ns = frame_args.t.tv_sec * 1000000000 + frame_args.t.tv_nsec;
//#endif

        auto img_frame = image->collect_frame();
        if (img_frame == nullptr || !img_frame->available()) {
            return;
        }

        std::shared_ptr<mnn> mnn = mnns->get_mnn(std::this_thread::get_id());
        if (mnn != nullptr) {
            std::vector<cv::Rect> faces;
            mnn->detect_faces(img_frame, faces);
            mnn->flag_faces(img_frame, faces);
//            log_d("face detect count: %ld.", faces.size());
        }

        if (image->is_recording()) {
#if LOG_ABLE && LOG_DRAW_TIME
            struct timespec et{0,0};
            clock_gettime(CLOCK_REALTIME, &et);
#endif
            bool aud_changed;
            auto aud_frame = audio->collect_frame(&aud_changed);
            aud_changed = aud_frame && aud_frame->available() && aud_changed;
            ffmpeg::video_encode_frame(image->get_name(),
                                       {image->get_width(),image->get_height(),image->get_channels(),(uint32_t) image->get_fps()},
                                       {audio->get_channels(),audio->get_sample_rate(),audio->get_frame_size()},
                                       std::shared_ptr<media::ffmpeg>(ffmpeg),
                                       std::make_shared<media::image_frame>(*img_frame),
                                       aud_changed?std::make_shared<media::audio_frame>(*aud_frame): nullptr);
#if LOG_ABLE && LOG_DRAW_TIME
            ens = et.tv_sec * 1000000000 + et.tv_nsec;
            clock_gettime(CLOCK_REALTIME, &et);
            ens = et.tv_sec * 1000000000 + et.tv_nsec - ens;
#endif
        } else {
            ffmpeg::video_encode_idle(std::shared_ptr<media::ffmpeg>(ffmpeg));
        }

#if LOG_ABLE && LOG_DRAW_TIME
        long sns = st.tv_sec * 1000000000 + st.tv_nsec;
        clock_gettime(CLOCK_REALTIME, &st);
        sns = st.tv_sec * 1000000000 + st.tv_nsec - sns;
//        log_d("record_collect time: %.2fms, %.2fms.", ens / (float)1000000.0f, sns / (float)1000000.0f);
#endif
        if (shw_frame != nullptr) {
            std::lock_guard<std::mutex> lock(shw_frame->get_mutex());
            shw_frame->set_frame(std::make_shared<media::image_frame>(*img_frame));
        }
    }

private:
    std::shared_ptr<show_frame> shw_frame;
    std::shared_ptr<ffmpeg> ffmpeg;
    std::shared_ptr<mnns> mnns;
    std::shared_ptr<image_recorder> image;
    std::shared_ptr<audio_recorder> audio;
};

} //namespace media

/*
 * run in renderer thread.
 */
void media::common::renderer_draw_frame() {
    if (loop_collect_count() < 24) {
        auto *ctx = new record_collect_ctx(
                std::forward<std::shared_ptr<media::show_frame>>(shw_frame),
                std::forward<std::shared_ptr<media::ffmpeg>>(ffmpeg),
                std::forward<std::shared_ptr<media::mnns>>(mnns),
                std::forward<std::shared_ptr<image_recorder>>(img_recorder),
                std::forward<std::shared_ptr<audio_recorder>>(aud_recorder));
        loop_post_collect([](void *ctx, void(*callback)(void *)) {
            ((record_collect_ctx *) ctx)->run();
            delete ((record_collect_ctx *) ctx);
        }, ctx, nullptr);
    }
    if (renderer != nullptr && shw_frame != nullptr) {
        renderer->draw_frame(shw_frame->get_frame());
    }
#if LOG_ABLE && LOG_DRAW_TIME
    struct timespec st{0,0};
    clock_gettime(CLOCK_REALTIME, &st);
    long sns = st.tv_sec * 1000000000 + st.tv_nsec - draw_t;
    draw_t = st.tv_sec * 1000000000 + st.tv_nsec;
//    log_d("renderer_draw_frame time: %.2fms.", sns / (float)1000000.0f);
#endif
}

/*
 * run in renderer thread.
 */
void media::common::renderer_select_camera(int32_t camera) {
    if (img_recorder != nullptr) {
        img_recorder->select_camera(camera);
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_record_start(std::string &&name) {
    if (img_recorder != nullptr && img_recorder->is_previewing()) {
        img_recorder->set_name(std::forward<std::string>(name));
        img_recorder->set_recording(true);
    }
    if (aud_recorder != nullptr) {
        aud_recorder->start_record();
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_record_stop() {
    if (img_recorder != nullptr) {
        img_recorder->set_recording(false);
    }
    if (aud_recorder != nullptr) {
        aud_recorder->stop_record();
    }
}

/*
 * run in caller thread.
 */
bool media::common::renderer_record_running() {
    return img_recorder != nullptr && img_recorder->is_recording();
}
