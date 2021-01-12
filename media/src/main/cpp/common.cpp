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

media::common::common(std::string &cascade, std::string &mnn)
:frame_args(), cascade(cascade), mnn(mnn), recorder(nullptr), renderer(nullptr) {
    log_d("created. cascade:%s, mnn:%s", cascade.c_str(), mnn.c_str());
}

media::common::~common() {
    delete recorder;
    delete renderer;
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
void media::common::renderer_surface_created() {
    delete recorder;
    recorder = new image_recorder();
    if (renderer) {
        renderer->surface_created();
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_destroyed() {
    if (renderer) {
        renderer->surface_destroyed();
    }
    delete recorder;
    recorder = nullptr;
}

/*
 * run in renderer thread.
 */
void media::common::renderer_surface_changed(int32_t w, int32_t h) {
    if (renderer) {
        renderer->surface_changed(w, h);
    }
    if (recorder) {
        recorder->update_size(w, h);
    }
}

/*
 * run in renderer thread.
 */
void media::common::renderer_draw_frame() {
    if (recorder == nullptr || renderer == nullptr) {
        return;
    }

#if LOG_ABLE
    clock_gettime(CLOCK_REALTIME, &frame_args.t);
    frame_args.ns = frame_args.t.tv_sec * 1000000000 + frame_args.t.tv_nsec;
#endif

    std::shared_ptr<media::image_cache> frame = recorder->update_frame();
    if (frame == nullptr || !frame->available()) {
        return;
    }

    frame->get(&frame_args.frame_width, &frame_args.frame_height, &frame_args.frame_cache);
    if (frame_args.frame_cache == nullptr) {
        return;
    }

    renderer->draw_frame(frame_args.frame_width, frame_args.frame_height, frame_args.frame_cache);

#if LOG_ABLE
    clock_gettime(CLOCK_REALTIME, &frame_args.t);
    frame_args.d_ns = frame_args.t.tv_sec * 1000000000 + frame_args.t.tv_nsec - frame_args.ns;
    frame_args.ns = frame_args.t.tv_sec * 1000000000 + frame_args.t.tv_nsec;
    log_d("draw frame use ms: %.3f", (float)frame_args.d_ns / 1000000.0f);
#endif
}
