//
// Created by Scliang on 4/13/21.
//

#include "jni_log.h"
#include "face_paint.h"

#define log_d(...)  LOG_D("Media-Native:face_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:face_paint", __VA_ARGS__)

namespace media {
} //namespace media

media::face_paint::face_paint(std::string &froot):fbo_paint(froot),
k_face_x(), k_face_y(), k_face_z(), k_face_w() {
    log_d("created.");
}

media::face_paint::~face_paint() {
    log_d("release.");
}

std::string media::face_paint::gen_effect_frag_shader_str() {
    return read_shader_str("shader_frag_effect_face.glsl");
}

void media::face_paint::on_setup_program_args(GLuint prog, const image_frame &frame) {
    int32_t width = 0, height = 0;
    frame.get(&width, &height, nullptr);
    std::vector<cv::Rect> fs;
    frame.get_faces(fs);
    cv::Rect face;
    if (!fs.empty()) { face = fs[0]; }

    setVec2(prog, "u_TexSize", glm::vec2(width, height));
    setInt(prog, "u_FaceCount", fs.size());
    if (fs.empty()) {
        setVec4(prog, "u_FaceRect", glm::vec4(0, 0, 0, 0));
    } else {
        setVec4(prog, "u_FaceRect", glm::vec4(
                k_face_x.filter(face.x), k_face_y.filter(face.y),
                k_face_z.filter((float) face.x + face.width),
                k_face_w.filter((float) face.y + face.height)));
    }
}
