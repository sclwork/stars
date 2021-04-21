//
// Created by Scliang on 4/16/21.
//

#include "jni_log.h"
#include "flame_paint.h"

#define log_d(...)  LOG_D("Media-Native:flame_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:flame_paint", __VA_ARGS__)

namespace media {
} //namespace media

media::flame_paint::flame_paint(std::string &froot):fbo_paint(froot), f_index(0) {
    log_d("created.");
}

media::flame_paint::~flame_paint() {
    log_d("release.");
}

std::string media::flame_paint::gen_effect_frag_shader_str() {
//    return read_shader_str("shader_frag_effect_flame.glsl");
    return read_shader_str("shader_frag_effect_burn.glsl");
}

void media::flame_paint::on_setup_program_args(GLuint prog, const image_frame &frame) {
    int32_t width = 0, height = 0;
    frame.get(&width, &height, nullptr);
    std::vector<cv::Rect> fs;
    frame.get_faces(fs);
    cv::Rect face;
    if (!fs.empty()) { face = fs[0]; }

    setVec2(prog, "u_TexSize", glm::vec2(width, height));
    setFloat(prog, "u_Time", f_index / 16.0f);

    f_index++;
    if (f_index == INT32_MAX) f_index = 0;
}
