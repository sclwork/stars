//
// Created by Scliang on 4/13/21.
//

#include "jni_log.h"
#include "opencv.h"
#include "lut_paint.h"

#define log_d(...)  LOG_D("Media-Native:lut_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:lut_paint", __VA_ARGS__)

media::lut_paint::lut_paint(std::string &froot):fbo_paint(),
file_root(froot), lut_texture(GL_NONE), lut_wid(0), lut_hei(0), lut_img(nullptr) {
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

const char *media::lut_paint::gen_effect_frag_shader_str() {
    return "#version 300 es                                                  \n"
           "precision highp float;                                           \n"
           "in vec2 v_texCoord;                                              \n"
           "layout(location = 0) out vec4 outColor;                          \n"
           "uniform sampler2D s_Texture;                                     \n"
           "uniform sampler2D s_LutTexture;                                  \n"
           "vec4 lut_filter(vec2 texCoord)                                   \n"
           "{                                                                \n"
           "    vec4 textureColor = texture(s_Texture, texCoord);            \n"
           "    float blueColor = textureColor.b * 63.0;                     \n"
           "    vec2 quad1;                                                  \n"
           "    quad1.y = floor(floor(blueColor) / 8.0);                     \n"
           "    quad1.x = floor(blueColor) - (quad1.y * 8.0);                \n"
           "    vec2 quad2;                                                  \n"
           "    quad2.y = floor(ceil(blueColor) / 7.9999);                   \n"
           "    quad2.x = ceil(blueColor) - (quad2.y * 8.0);                 \n"
           "    vec2 texPos1;                                                \n"
           "    texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);\n"
           "    texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);\n"
           "    vec2 texPos2;                                                \n"
           "    texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);\n"
           "    texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);\n"
           "    vec4 newColor1 = texture(s_LutTexture, texPos1);             \n"
           "    vec4 newColor2 = texture(s_LutTexture, texPos2);             \n"
           "    vec4 newColor = mix(newColor1, newColor2, fract(blueColor)); \n"
           "    return mix(textureColor, vec4(newColor.rgb, textureColor.w), 1.0);                 \n"
           "}                                                                \n"
           "void main()                                                      \n"
           "{                                                                \n"
           "    outColor = lut_filter(v_texCoord);                           \n"
           "}";
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
