//
// Created by scliang on 1/7/21.
//

#include "jni_log.h"
#include "fbo_paint.h"
#include "lut_paint.h"
#include "face_paint.h"
#include "ripple_paint.h"
#include "flame_paint.h"
#include "effect_selector_paint.h"
#include "image_renderer.h"

#define log_d(...)  LOG_D("Media-Native:image_renderer", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:image_renderer", __VA_ARGS__)

namespace media {
} //namespace media

media::image_renderer::image_renderer(std::string &froot,
                                      moodycamel::ConcurrentQueue<image_frame> &iQ,
                                      bool (*cvrecording)())
:file_root(froot), width(0), height(0), paint(nullptr),
eiQ(iQ), drawQ(), check_video_recording(cvrecording) {
    log_d("created.");
}

media::image_renderer::~image_renderer() {
    delete paint;
    log_d("release.");
}

void media::image_renderer::surface_created(const std::string &effect) {
    delete paint;
    paint = create_paint(effect);
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
    of.set_ori(nf.get_ori());

    if (check_video_recording != nullptr && check_video_recording() && of.available()) {
        eiQ.enqueue(std::forward<image_frame>(of));
    }
}

void media::image_renderer::clear_frame() {
    media::paint *t = paint;
    paint = nullptr;
    image_frame f;
    while (drawQ.try_dequeue(f));
    paint = t;
}

void media::image_renderer::updt_paint(const std::string &name) {
    clear_frame();
    delete paint;
    paint = create_paint(name);
}

media::paint *media::image_renderer::create_paint(const std::string &name) {
    log_d("create effect paint name: %s.", name.c_str());
    media::paint *p;
    if (name == "NONE") {
        p = new fbo_paint(file_root);
    } else if (name == "FACE") {
        p = new face_paint(file_root);
    } else if (name == "RIPPLE") {
        p = new ripple_paint(file_root);
    } else if (name == "3BASIC") {
        p = new effect_selector_paint(file_root, EFFECT_3Basic);
    } else if (name == "DISTORTEDTV") {
        p = new effect_selector_paint(file_root, EFFECT_Distortedtv);
    } else if (name == "DISTORTEDTV_BOX") {
        p = new effect_selector_paint(file_root, EFFECT_DistortedtvBox);
    } else if (name == "DISTORTEDTV_CRT") {
        p = new effect_selector_paint(file_root, EFFECT_DistortedtvCRT);
    } else if (name == "DISTORTEDTV_GLITCH") {
        p = new effect_selector_paint(file_root, EFFECT_DistortedtvGlitch);
    } else if (name == "EB") {
        p = new effect_selector_paint(file_root, EFFECT_EdgesBilateral);
    } else if (name == "SIN_WAVE") {
        p = new effect_selector_paint(file_root, EFFECT_SinWave);
    } else if (name == "FLOYD") {
        p = new effect_selector_paint(file_root, EFFECT_Floyd);
    } else if (name == "3FLOYD") {
        p = new effect_selector_paint(file_root, EFFECT_3Floyd);
    } else if (name == "PAGECURL") {
        p = new effect_selector_paint(file_root, EFFECT_PageCurl);
    } else if (name == "OLD_VIDEO") {
        p = new effect_selector_paint(file_root, EFFECT_OldVideo);
    } else if (name == "CROSSHATCH") {
        p = new effect_selector_paint(file_root, EFFECT_Crosshatch);
    } else if (name == "CMYK") {
        p = new effect_selector_paint(file_root, EFFECT_CMYK);
    } else if (name == "DRAWING") {
        p = new effect_selector_paint(file_root, EFFECT_Drawing);
    } else if (name == "NEON") {
        p = new effect_selector_paint(file_root, EFFECT_Neon);
    } else if (name == "FISHEYE") {
        p = new effect_selector_paint(file_root, EFFECT_Fisheye);
    } else if (name == "BARRELBLUR") {
        p = new effect_selector_paint(file_root, EFFECT_BarrelBlur);
    } else if (name == "FASTBLUR") {
        p = new effect_selector_paint(file_root, EFFECT_FastBlur);
    } else if (name == "ILLUSTRATION") {
        p = new effect_selector_paint(file_root, EFFECT_Illustration);
    } else if (name == "HEXAGON") {
        p = new effect_selector_paint(file_root, EFFECT_Hexagon);
    } else if (name == "SOBEL") {
        p = new effect_selector_paint(file_root, EFFECT_Sobel);
    } else if (name == "LENS") {
        p = new effect_selector_paint(file_root, EFFECT_Lens);
    } else if (name == "FLOAT_CAMERA") {
        p = new effect_selector_paint(file_root, EFFECT_FloatCamera);
    } else {
        p = new fbo_paint(file_root);
    }
    p->set_canvas_size(width, height);
    return p;
}
