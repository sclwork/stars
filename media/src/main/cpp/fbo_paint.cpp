//
// Created by Scliang on 3/30/21.
//

#include <glm/vec2.hpp>
#include "jni_log.h"
#include "fbo_paint.h"

#define log_d(...)  LOG_D("Media-Native:fbo_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:fbo_paint", __VA_ARGS__)

namespace media {

static GLfloat vcs[] = {
        -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
};

static GLfloat tcs[] = {
        0.0f,  0.0f,
        0.0f,  1.0f,
        1.0f,  1.0f,
        1.0f,  0.0f
};

static GLfloat mirror_tcs[] = {
        1.0f,  0.0f,
        1.0f,  1.0f,
        0.0f,  1.0f,
        0.0f,  0.0f,
};

static GLushort indices[] = {
        0, 1, 2,
        0, 2, 3
};

} //namespace media

media::fbo_paint::fbo_paint(std::string &froot):gl_paint(froot),
cvs_width(0), cvs_height(0), cvs_ratio(0),
program(GL_NONE), effect_program(GL_NONE), texture(GL_NONE),
src_fbo(GL_NONE), src_fbo_texture(GL_NONE), dst_fbo(GL_NONE), dst_fbo_texture(GL_NONE) {
    log_d("created.");
}

media::fbo_paint::~fbo_paint() {
    if (program != GL_NONE) glDeleteProgram(program);
    if (effect_program != GL_NONE) glDeleteProgram(effect_program);
    if (texture != GL_NONE) glDeleteTextures(1, &texture);
    if (src_fbo_texture != GL_NONE) glDeleteTextures(1, &src_fbo_texture);
    if (src_fbo != GL_NONE) glDeleteFramebuffers(1, &src_fbo);
    if (dst_fbo_texture != GL_NONE) glDeleteTextures(1, &dst_fbo_texture);
    if (dst_fbo != GL_NONE) glDeleteFramebuffers(1, &dst_fbo);
    log_d("release.");
}

void media::fbo_paint::set_canvas_size(int32_t width, int32_t height) {
    if (height == 0) {
        return;
    }

    cvs_width = width;
    cvs_height = height;
    cvs_ratio = (float)width/(float)height;
    log_d("canvas size: %d,%d %0.4f", cvs_width, cvs_height, cvs_ratio);
    glViewport(0, 0, cvs_width, cvs_height);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    glGenTextures(1, &src_fbo_texture);
    glBindTexture(GL_TEXTURE_2D, src_fbo_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    glGenTextures(1, &dst_fbo_texture);
    glBindTexture(GL_TEXTURE_2D, dst_fbo_texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    glGenFramebuffers(1, &src_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, src_fbo);
    glBindTexture(GL_TEXTURE_2D, src_fbo_texture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src_fbo_texture, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE) {
        log_e("create src_fbo fail.");
        if(src_fbo_texture != GL_NONE) {
            glDeleteTextures(1, &src_fbo_texture);
            src_fbo_texture = GL_NONE;
        }
        if(src_fbo != GL_NONE) {
            glDeleteFramebuffers(1, &src_fbo);
            src_fbo = GL_NONE;
        }
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    }

    glGenFramebuffers(1, &dst_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, dst_fbo);
    glBindTexture(GL_TEXTURE_2D, dst_fbo_texture);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst_fbo_texture, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER)!= GL_FRAMEBUFFER_COMPLETE) {
        log_e("create dst_fbo fail.");
        if(dst_fbo_texture != GL_NONE) {
            glDeleteTextures(1, &dst_fbo_texture);
            dst_fbo_texture = GL_NONE;
        }
        if(dst_fbo != GL_NONE) {
            glDeleteFramebuffers(1, &dst_fbo);
            dst_fbo = GL_NONE;
        }
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    }

    log_d("create src_fbo/dst_fbo success.");

    program = create_program(gen_vert_shader_str().c_str(), gen_frag_shader_str().c_str());
    effect_program = create_program(gen_effect_vert_shader_str().c_str(), gen_effect_frag_shader_str().c_str());

    on_canvas_size_changed(width, height);
}

void media::fbo_paint::draw(const image_frame &frame, image_frame *of) {
    if (!frame.available()) {
        return;
    }

    bool mirror = frame.use_mirror();
    int32_t width = 0, height = 0;
    uint32_t *data = nullptr;
    frame.get(&width, &height, &data);

    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(effect_program == GL_NONE || src_fbo == GL_NONE || data == nullptr) {
        return;
    }

    on_pre_tex_image(width, height, data);

    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

    // 渲染到 FBO
    glBindFramebuffer(GL_FRAMEBUFFER, src_fbo);
    glViewport(0, 0, width, height);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(effect_program);
    glBindTexture(GL_TEXTURE_2D, src_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, dst_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), vcs);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), mirror ? mirror_tcs : tcs);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glm::mat4 matrix;
    on_update_matrix(matrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    setInt(effect_program, "s_Texture", 0);
    setMat4(effect_program, "u_MVPMatrix", matrix);
    on_setup_program_args(effect_program, frame);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

    // 再绘制一次，把方向倒过来
    glBindFramebuffer(GL_FRAMEBUFFER, dst_fbo);
    glViewport(0, 0, width, height);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (program);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), vcs);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), mirror ? mirror_tcs : tcs);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindTexture(GL_TEXTURE_2D, src_fbo_texture);
    setInt(program, "s_Texture", 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    if (of != nullptr) gl_pixels_to_image_frame(of, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

    // 渲染到屏幕
    glViewport(0, 0, cvs_width, cvs_height);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dst_fbo_texture);
    setInt(program, "s_Texture", 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

void media::fbo_paint::update_matrix(glm::mat4 &matrix, int32_t angleX, int32_t angleY, float scaleX, float scaleY) {
    angleX = angleX % 360;
    angleY = angleY % 360;

    auto radiansX = MATH_PI / 180.0f * angleX;
    auto radiansY = MATH_PI / 180.0f * angleY;

    // Projection matrix
//    glm::mat4 Projection = glm::ortho(-cvs_ratio, cvs_ratio, -1.0f, 1.0f, 0.0f, 100.0f);
    glm::mat4 Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    //glm::mat4 Projection = glm::frustum(-ratio, ratio, -1.0f, 1.0f, 4.0f, 100.0f);
//    glm::mat4 Projection = glm::perspective(45.0f,cvs_ratio, 0.1f,100.f);

    // View matrix
    glm::mat4 View = glm::lookAt(
            glm::vec3(0, 0, 4), // Camera is at (0,0,1), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, glm::vec3(scaleX, scaleY, 1.0f));
    Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
    Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
    Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

    matrix = Projection * View * Model;
}

void media::fbo_paint::gl_pixels_to_image_frame(
        media::image_frame *of, int32_t width, int32_t height) {
    uint32_t *of_data = nullptr;
    if (of->available()) {
        if (of->same_size(width, height)) {
            of->get(nullptr, nullptr, &of_data);
        } else {
            of->update_size(width, height);
            of->get(nullptr, nullptr, &of_data);
        }
    } else {
        of->update_size(width, height);
        of->get(nullptr, nullptr, &of_data);
    }
    if (of_data != nullptr) {
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, of_data);
    }
}

std::string media::fbo_paint::gen_vert_shader_str() {
    return read_shader_str("shader_vert_none.glsl");
}

std::string media::fbo_paint::gen_frag_shader_str() {
    return read_shader_str("shader_frag_none.glsl");
}

std::string media::fbo_paint::gen_effect_vert_shader_str() {
    return read_shader_str("shader_vert_effect_none.glsl");
}

std::string media::fbo_paint::gen_effect_frag_shader_str() {
    return read_shader_str("shader_frag_effect_none.glsl");
}

void media::fbo_paint::on_update_matrix(glm::mat4 &matrix) {
    update_matrix(matrix, 0, 0, 1.0, 1.0);
}

void media::fbo_paint::on_pre_tex_image(int32_t width, int32_t height, uint32_t *data) {}
void media::fbo_paint::on_setup_program_args(GLuint prog, const image_frame &frame) {}
void media::fbo_paint::on_canvas_size_changed(int32_t width, int32_t height) {}
