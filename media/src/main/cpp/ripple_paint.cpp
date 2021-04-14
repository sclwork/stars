//
// Created by Scliang on 4/14/21.
//

#include "jni_log.h"
#include "ripple_paint.h"

#define log_d(...)  LOG_D("Media-Native:ripple_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:ripple_paint", __VA_ARGS__)

namespace media {
} //namespace media

media::ripple_paint::ripple_paint():fbo_paint(), rfx(), rfy(),
f_index(0), k_face_x(), k_face_y(), k_face_z(), k_face_w() {
    srandom((unsigned)time(nullptr));
    log_d("created.");
}

media::ripple_paint::~ripple_paint() {
    log_d("release.");
}

const char *media::ripple_paint::gen_effect_frag_shader_str() {
    return "#version 300 es                                                  \n"
           "precision highp float;                                           \n"
           "in vec2 v_texCoord;                                              \n"
           "layout(location = 0) out vec4 outColor;                          \n"
           "uniform sampler2D s_Texture;                                     \n"
           "uniform vec2 u_TexSize;                                          \n"
           "uniform int u_FaceCount;                                         \n"
           "uniform vec4 u_FaceRect;                                         \n"
           "uniform float u_Time;                                            \n"
           "uniform float u_Boundary;                                        \n"
           "uniform vec4 u_RPoint;                                           \n"
           "uniform vec2 u_ROffset;                                          \n"
           "vec2 ripple(vec2 tc, float of, float cx, float cy) {             \n"
           "    float ratio = u_TexSize.y / u_TexSize.x;                     \n"
           "    vec2 texCoord = tc * vec2(1.0, ratio);                       \n"
           "    vec2 touchXY = vec2(cx, cy) * vec2(1.0, ratio);              \n"
           "    float distance = distance(texCoord, touchXY);                \n"
           "    if ((u_Time - u_Boundary) > 0.0                              \n"
           "    && (distance <= (u_Time + u_Boundary))                       \n"
           "    && (distance >= (u_Time - u_Boundary))) {                    \n"
           "        float x = (distance - u_Time);                           \n"
           "        float moveDis=of*x*(x-u_Boundary)*(x+u_Boundary);        \n"
           "        vec2 unitDirectionVec = normalize(texCoord - touchXY);   \n"
           "        texCoord = texCoord + (unitDirectionVec * moveDis);      \n"
           "    }                                                            \n"
           "    texCoord = texCoord / vec2(1.0, ratio);                      \n"
           "    return texCoord;                                             \n"
           "}                                                                \n"
           "void main()                                                      \n"
           "{                                                                \n"
           "    float fx = u_FaceRect.x / u_TexSize.x;                       \n"
           "    float fy = u_FaceRect.y / u_TexSize.y;                       \n"
           "    float fz = u_FaceRect.z / u_TexSize.x;                       \n"
           "    float fw = u_FaceRect.w / u_TexSize.y;                       \n"
           "    float cx = (fz + fx) / 2.0;                                  \n"
           "    float cy = (fw + fy) / 2.0;                                  \n"
           "    vec2 tc = ripple(v_texCoord, 20.0, cx, cy);                  \n"
           "    tc=ripple(tc,u_ROffset.x,u_RPoint.x/u_TexSize.x,u_RPoint.y/u_TexSize.y); \n"
           "    tc=ripple(tc,u_ROffset.y,u_RPoint.z/u_TexSize.x,u_RPoint.w/u_TexSize.y); \n"
           "    outColor = texture(s_Texture, tc);                           \n"
           "}";
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
