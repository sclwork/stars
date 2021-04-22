//
// Created by Scliang on 4/21/21.
//

#include "jni_log.h"
#include "opencv.h"
#include "distortedtv_paint.h"

#define log_d(...)  LOG_D("Media-Native:distortedtv_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:distortedtv_paint", __VA_ARGS__)

namespace media {
} //namespace media

media::distortedtv_paint::distortedtv_paint(std::string &froot, bool grey)
:fbo_paint(froot), use_grey(grey), f_index(0) {
    log_d("created.");
}

media::distortedtv_paint::~distortedtv_paint() {
    log_d("release.");
}

std::string media::distortedtv_paint::gen_effect_frag_shader_str() {
//    return read_shader_str("shader_frag_effect_distortedtv.glsl");
//    return read_shader_str("shader_frag_effect_distortedtv_box.glsl");
    return read_shader_str("shader_frag_effect_distortedtv_glitch.glsl");
}

void media::distortedtv_paint::on_pre_tex_image(int32_t width, int32_t height, uint32_t *data) {
    if (use_grey && data != nullptr) {
        opencv::grey_data(width, height, data);
    }
}

void media::distortedtv_paint::on_setup_program_args(GLuint prog, const image_frame &frame) {
    int32_t width = 0, height = 0;
    frame.get(&width, &height, nullptr);
    std::vector<cv::Rect> fs;
    frame.get_faces(fs);
    cv::Rect face;
    if (!fs.empty()) { face = fs[0]; }

    setVec2(prog, "u_TexSize", glm::vec2(width, height));
    setFloat(prog, "u_Time", f_index / 64.0f);
    setBool(prog, "u_Mirror", frame.use_mirror());

    f_index++;
    if (f_index == INT32_MAX) f_index = 0;
}
