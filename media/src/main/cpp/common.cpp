//
// Created by scliang on 1/6/21.
//

#include "common.h"
#include "video/collect/image_recorder.h"
#include "video/play/image_renderer.h"

#define log_d(...)  LOG_D("Media-Native:common", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:common", __VA_ARGS__)

namespace media {
} //namespace media

media::common::common(std::string &cas_path, std::string &mnn_path)
:frame_args(), cas_path(cas_path), renderer(nullptr),
img_recorder(nullptr), aud_recorder(nullptr),
#if IMPORT_TFLITE
tflite(new media::tflite()),
#endif
mnn(new media::mnn(mnn_path, 4)), ffmpeg(new media::ffmpeg()),
record_state(RECORD_STATE::NONE) {
#if LOG_ABLE && LOG_DRAW_TIME
    frame_args.fps_count = 0;
    frame_args.fps_sum = 0;
#endif
    log_d("created.");
//    log_d("created. cascade:%s.", cas_path.c_str());
}

media::common::~common() {
    delete renderer;
    delete img_recorder;
    delete aud_recorder;
    log_d("release.");
}

/*
 * run in renderer thread.
 */
void media::common::renderer_init() {
    delete renderer;
    renderer = new image_renderer();
}

/*
 * run in renderer thread.
 */
void media::common::renderer_release() {
    delete renderer;
    renderer = nullptr;
}

/*
 * run in renderer thread.
 */
int32_t media::common::renderer_surface_created() {
    delete aud_recorder;
    aud_recorder = new audio_recorder();
    delete img_recorder;
    img_recorder = new image_recorder();
    if (renderer) {
        renderer->surface_created();
    }
    return img_recorder ? img_recorder->camera_count() : 0;
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_destroyed() {
    if (renderer) {
        renderer->surface_destroyed();
    }
    delete img_recorder;
    img_recorder = nullptr;
    delete aud_recorder;
    aud_recorder = nullptr;
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_changed(int32_t w, int32_t h) {
    if (renderer) {
        renderer->surface_changed(w, h);
    }
    if (img_recorder) {
        img_recorder->update_size(w, h);
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_draw_frame() {
    if (img_recorder == nullptr || renderer == nullptr) {
        return;
    }

#if LOG_ABLE && LOG_DRAW_TIME
    clock_gettime(CLOCK_REALTIME, &frame_args.t);
    frame_args.ns = frame_args.t.tv_sec * 1000000000 + frame_args.t.tv_nsec;
#endif

    auto frame = img_recorder->collect_frame();
    if (frame == nullptr || !frame->available()) {
        return;
    }

    mnn->face_detect(frame, frame_args.faces);
    mnn->flag_faces(frame, frame_args.faces, frame_args.fps);
//    log_d("face detect count: %ld.", frame_args.faces.size());

    renderer->draw_frame(frame);

    if (record_state == RECORD_STATE::RECORDING) {
        ffmpeg->encode_frame(std::shared_ptr<media::image_frame>(
                new media::image_frame(*frame)));
    }

#if LOG_ABLE && LOG_DRAW_TIME
    clock_gettime(CLOCK_REALTIME, &frame_args.t);
    frame_args.d_ns = frame_args.t.tv_sec * 1000000000 + frame_args.t.tv_nsec - frame_args.ns;
    frame_args.ns = frame_args.t.tv_sec * 1000000000 + frame_args.t.tv_nsec;
//    log_d("draw frame use ms: %.3f, %.3ffps",
//            (float)frame_args.d_ns / 1000000.0f,
//            1000.0f / ((float)frame_args.d_ns / 1000000.0f));
    frame_args.fps_sum += (int32_t)(1000.0f / ((float)frame_args.d_ns / 1000000.0f));
    frame_args.fps_count ++;
    if (frame_args.fps_count == 20) {
        frame_args.fps = std::to_string(frame_args.fps_sum / frame_args.fps_count) + "fps";
        frame_args.fps_sum = 0;
        frame_args.fps_count = 0;
    }
#endif
}

/*
 * run in renderer thread.
 */
void media::common::renderer_select_camera(int camera) {
    if (img_recorder) {
        img_recorder->select_camera(camera);
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_record_start(std::string &&name) {
    if (img_recorder && img_recorder->is_previewing()) {
        log_d("record start. %s.", name.c_str());
        if (aud_recorder) {
            aud_recorder->start_record();
        }
        record_state = RECORD_STATE::RECORDING;
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_record_stop() {
    record_state = RECORD_STATE::NONE;
    if (aud_recorder) {
        aud_recorder->stop_record();
    }
}

/*
 * run in caller thread.
 */
bool media::common::renderer_record_running() {
    return record_state == RECORD_STATE::RECORDING;
}
