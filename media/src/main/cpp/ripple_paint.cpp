//
// Created by Scliang on 4/14/21.
//

#include "jni_log.h"
#include "ripple_paint.h"

#define log_d(...)  LOG_D("Media-Native:ripple_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:ripple_paint", __VA_ARGS__)

namespace media {
} //namespace media

media::ripple_paint::ripple_paint(std::string &froot):fbo_paint(froot), rfx(), rfy(),
f_index(0), k_face_x(), k_face_y(), k_face_z(), k_face_w() {
    srandom((unsigned)time(nullptr));
    log_d("created.");
}

media::ripple_paint::~ripple_paint() {
    log_d("release.");
}

std::string media::ripple_paint::gen_effect_frag_shader_str() {
    return read_shader_str("shader_frag_effect_ripple.glsl");
}

void media::ripple_paint::on_setup_program_args(GLuint prog, const image_frame &frame) {
    int32_t width = 0, height = 0;
    frame.get(&width, &height, nullptr);
    std::vector<cv::Rect> fs;
    frame.get_faces(fs);
    cv::Rect face;
    if (!fs.empty()) { face = fs[0]; }

    auto time = (float)(fmod(f_index, 200) / 160);
    if (time == 0.0) {
        rfx[0] = random() % width;
        rfy[0] = random() % height;
        rfx[1] = random() % width;
        rfy[1] = random() % height;
        rfx[2] = random() % width;
        rfy[2] = random() % height;
    }

    setVec2(prog, "u_TexSize", glm::vec2(width, height));
    setInt(prog, "u_FaceCount", fs.size());
    if (fs.empty()) {
        setVec4(prog, "u_FaceRect", glm::vec4(
                rfx[0], rfy[0], rfx[0] + 80, rfy[0] + 80));
    } else {
        setVec4(prog, "u_FaceRect", glm::vec4(
                k_face_x.filter(face.x), k_face_y.filter(face.y),
                k_face_z.filter((float) face.x + face.width),
                k_face_w.filter((float) face.y + face.height)));
    }

    setVec4(prog, "u_RPoint", glm::vec4(
            rfx[1] + 40, rfy[1] + 40, rfx[2] + 40, rfy[2] + 40));
    setVec2(prog, "u_ROffset", glm::vec2(
            10 + random() % 10, 10 + random() % 10));
    setFloat(prog, "u_Time", time * 2.5f);
    setFloat(prog, "u_Boundary", 0.12);

    f_index++;
    if (f_index == INT32_MAX) f_index = 0;
}
