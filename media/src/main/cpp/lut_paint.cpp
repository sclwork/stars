//
// Created by Scliang on 4/13/21.
//

#include "jni_log.h"
#include "opencv.h"
#include "lut_paint.h"

#define log_d(...)  LOG_D("Media-Native:lut_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:lut_paint", __VA_ARGS__)

media::lut_paint::lut_paint(std::string &froot):fbo_paint(froot),
lut_texture(GL_NONE), lut_wid(0), lut_hei(0), lut_img(nullptr) {
    if (!file_root.empty()) {
        lut_img = opencv::load_image(file_root + "/lut_a.png", &lut_wid, &lut_hei);
        log_d("lut img load success: %d, %d,%d.", lut_img!=nullptr, lut_wid, lut_hei);
    }
    log_d("created.");
}

media::lut_paint::~lut_paint() {
    if (lut_img != nullptr) free(lut_img);
    log_d("release.");
}

std::string media::lut_paint::gen_effect_frag_shader_str() {
    return read_shader_str("shader_frag_effect_lut.glsl");
}

void media::lut_paint::on_canvas_size_changed(int32_t width, int32_t height) {
    if (lut_img != nullptr) {
        glGenTextures(1, &lut_texture);
        glBindTexture(GL_TEXTURE_2D, lut_texture);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, lut_wid, lut_hei, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, lut_img);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }
}

void media::lut_paint::on_setup_program_args(GLuint prog, const image_frame &frame) {
    if (lut_texture != GL_NONE) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, lut_texture);
        setInt(prog, "s_LutTexture", 1);
    }
}
