//
// Created by Scliang on 3/30/21.
//

#include <glm/vec2.hpp>
#include "jni_log.h"
#include "opencv.h"
#include "fbo_shader.h"
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

media::fbo_paint::fbo_paint(std::string &froot)
:file_root(froot), cvs_width(0), cvs_height(0), cvs_ratio(0),
matrix(), program(GL_NONE), fbo_program(GL_NONE), texture(GL_NONE), lut_texture(GL_NONE),
vao(GL_NONE), vbo(), src_fbo(GL_NONE), src_fbo_texture(GL_NONE), dst_fbo(GL_NONE), dst_fbo_texture(GL_NONE),
frame_index(0), k_face_x(), k_face_y(), k_face_z(), k_face_w(), lut_wid(0), lut_hei(0), lut_img(nullptr) {
    if (!file_root.empty()) {
        lut_img = opencv::load_image(file_root + "/lut_a.png", &lut_wid, &lut_hei);
        log_d("lut img load success: %d, %d,%d.", lut_img!=nullptr, lut_wid, lut_hei);
    }
    log_d("created.");
}

media::fbo_paint::~fbo_paint() {
    if (lut_img != nullptr) free(lut_img);
    log_d("release.");
}

void media::fbo_paint::set_canvas_size(int32_t width, int32_t height) {
    if (height == 0) {
        return;
    }

    frame_index = 0;
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

    program = create_program(vShaderStr, fShaderStr);
    fbo_program = create_program(vShaderStr, fShaderStr);

    // Generate VAO Id
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Generate VBO Ids and load the VBOs with data
    glGenBuffers(3, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vcs), vcs, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tcs), tcs, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);

    glBindVertexArray(GL_NONE);
}

void media::fbo_paint::draw(const image_frame &frame, image_frame &of) {
    if (!frame.available()) {
        return;
    }

    bool mirror = frame.use_mirror();
    int32_t width = 0, height = 0;
    uint32_t *data = nullptr;
    frame.get(&width, &height, &data);
    std::vector<cv::Rect> fs;
    frame.get_faces(fs);
    cv::Rect face;
    if (!fs.empty()) { face = fs[0]; }

    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(program == GL_NONE || data == nullptr || src_fbo == GL_NONE) {
        return;
    }

    update_matrix(0, 0, 1.0, 1.0);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tcs), mirror ? mirror_tcs : tcs, GL_STATIC_DRAW);

    // 渲染到 FBO
    glBindFramebuffer(GL_FRAMEBUFFER, src_fbo);
    glViewport(0, 0, width, height);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(fbo_program);
    glBindTexture(GL_TEXTURE_2D, src_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, dst_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    setInt(fbo_program, "s_Texture", 0);
    if (lut_texture != GL_NONE) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, lut_texture);
        setInt(fbo_program, "s_LutTexture", 1);
    }
    setMat4(fbo_program, "u_MVPMatrix", matrix);
    setFloat(fbo_program, "u_Offset", (sin(frame_index * MATH_PI / 40) + 1.0f) / 2.0f);
    setVec2(fbo_program, "u_TexSize", glm::vec2(width, height));
    setInt(fbo_program, "u_FaceCount", fs.size());
    if (fs.empty()) {
        setVec4(fbo_program, "u_FaceRect", glm::vec4(0, 0, 0, 0));
    } else {
        setVec4(fbo_program, "u_FaceRect", glm::vec4(
                k_face_x.filter(face.x), k_face_y.filter(face.y),
                k_face_z.filter((float) face.x + face.width),
                k_face_w.filter((float) face.y + face.height)));
    }
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    // 再绘制一次，把方向倒过来
    glBindFramebuffer(GL_FRAMEBUFFER, dst_fbo);
    glViewport(0, 0, width, height);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (program);
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, src_fbo_texture);
    setInt(program, "s_Texture", 0);
    setMat4(program, "u_MVPMatrix", matrix);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    gl_pixels_to_image_frame(of, width, height);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

    // 渲染到屏幕
    glViewport(0, 0, cvs_width, cvs_height);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dst_fbo_texture);
    setInt(program, "s_Texture", 0);
    setMat4(program, "u_MVPMatrix", matrix);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    if (frame_index == INT32_MAX) {
        frame_index = 0;
    } else {
        frame_index++;
    }
}

void media::fbo_paint::update_matrix(int32_t angleX, int32_t angleY, float scaleX, float scaleY) {
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
        media::image_frame &of, int32_t width, int32_t height) {
    uint32_t *of_data = nullptr;
    if (of.available()) {
        if (of.same_size(width, height)) {
            of.get(nullptr, nullptr, &of_data);
        } else {
            of.update_size(width, height);
            of.get(nullptr, nullptr, &of_data);
        }
    } else {
        of.update_size(width, height);
        of.get(nullptr, nullptr, &of_data);
    }
    if (of_data != nullptr) {
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, of_data);
    }
}
