//
// Created by scliang on 1/7/21.
//

#include "jni_log.h"
#include "fbo_paint.h"
#include "lut_paint.h"
#include "face_paint.h"
#include "ripple_paint.h"
#include "flame_paint.h"
#include "camera_paint.h"
#include "image_renderer.h"

#define log_d(...)  LOG_D("Media-Native:image_renderer", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:image_renderer", __VA_ARGS__)

namespace media {
} //namespace media

media::image_renderer::image_renderer(std::string &froot,
                                      moodycamel::ConcurrentQueue<image_frame> &iQ,
                                      moodycamel::ConcurrentQueue<audio_frame> &aQ,
                                      bool (*cvrecording)())
:file_root(froot), width(0), height(0), paint(nullptr),
eiQ(iQ), eaQ(aQ), drawQ(), check_video_recording(cvrecording) {
    log_d("created.");
}

media::image_renderer::~image_renderer() {
    delete paint;
    log_d("release.");
}

void media::image_renderer::surface_created() {
    delete paint;
//    paint = new camera_paint(file_root);
//    paint = new fbo_paint(file_root);
//    paint = new lut_paint(file_root);
//    paint = new face_paint(file_root);
//    paint = new ripple_paint(file_root);
    paint = new flame_paint(file_root);
}

void media::image_renderer::surface_destroyed() {
    delete paint;
    paint = nullptr;
}

void media::image_renderer::surface_changed(int32_t w, int32_t h) {
    width = w;
    height = h;
    if (paint) {
        paint->set_canvas_size(w, h);
    }
}

void media::image_renderer::updt_frame(image_frame &&frm) {
    if (frm.available()) {
        drawQ.enqueue(std::forward<image_frame>(frm));
    }
}

void media::image_renderer::draw_frame() {
    if (paint == nullptr) {
        return;
    }

    image_frame of, nf;
    drawQ.try_dequeue(nf);
    paint->draw(nf, of);

    if (check_video_recording != nullptr && check_video_recording() && of.available()) {
        eiQ.enqueue(std::forward<image_frame>(of));
    }
}
