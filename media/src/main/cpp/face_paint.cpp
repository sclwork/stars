//
// Created by Scliang on 4/13/21.
//

#include "jni_log.h"
#include "face_paint.h"

#define log_d(...)  LOG_D("Media-Native:face_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:face_paint", __VA_ARGS__)

namespace media {
} //namespace media

media::face_paint::face_paint():fbo_paint(), k_face_x(), k_face_y(), k_face_z(), k_face_w() {
    log_d("created.");
}

media::face_paint::~face_paint() {
    log_d("release.");
}

const char *media::face_paint::gen_frag_shader_str() {
    return "#version 300 es                                                  \n"
           "precision highp float;                                           \n"
           "in vec2 v_texCoord;                                              \n"
           "layout(location = 0) out vec4 outColor;                          \n"
           "uniform sampler2D s_Texture;                                     \n"
           "uniform vec2 u_TexSize;                                          \n"
           "uniform int u_FaceCount;                                         \n"
           "uniform vec4 u_FaceRect;                                         \n"
           "void main()                                                      \n"
           "{                                                                \n"
           "    float fx = u_FaceRect.x / u_TexSize.x;                       \n"
           "    float fy = u_FaceRect.y / u_TexSize.y;                       \n"
           "    float fz = u_FaceRect.z / u_TexSize.x;                       \n"
           "    float fw = u_FaceRect.w / u_TexSize.y;                       \n"
           "    float cw = 0.5 / u_TexSize.x;                                \n"
           "    float ch = 0.5 / u_TexSize.y;                                \n"
           "    if (((v_texCoord.x > fx - cw && v_texCoord.x < fx + cw)      \n"
           "      || (v_texCoord.y > fy - ch && v_texCoord.y < fy + ch)      \n"
           "      || (v_texCoord.x > fz - cw && v_texCoord.x < fz + cw)      \n"
           "      || (v_texCoord.y > fw - ch && v_texCoord.y < fw + ch))     \n"
           "      && (v_texCoord.x > fx - cw && v_texCoord.x < fz + cw       \n"
           "       && v_texCoord.y > fy - ch && v_texCoord.y < fw + ch)      \n"
           "      && u_FaceCount > 0)                                        \n"
           "    {                                                            \n"
           "        outColor = vec4(1.0, 1.0, 1.0, 1.0);                     \n"
           "    }                                                            \n"
           "    else                                                         \n"
           "    {                                                            \n"
           "        outColor = texture(s_Texture, v_texCoord);               \n"
           "    }                                                            \n"
           "}";
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
