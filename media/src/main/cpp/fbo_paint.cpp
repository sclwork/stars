//
// Created by Scliang on 3/30/21.
//

#include <glm/vec2.hpp>
#include "jni_log.h"
#include "fbo_paint.h"

#define log_d(...)  LOG_D("Media-Native:fbo_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:fbo_paint", __VA_ARGS__)

namespace media {

static void setBool(GLuint programId, const std::string &name, bool value) {
    glUniform1i(glGetUniformLocation(programId, name.c_str()), (int) value);
}

static void setInt(GLuint programId, const std::string &name, int value) {
    glUniform1i(glGetUniformLocation(programId, name.c_str()), value);
}

static void setFloat(GLuint programId, const std::string &name, float value) {
    glUniform1f(glGetUniformLocation(programId, name.c_str()), value);
}

static void setVec2(GLuint programId, const std::string &name, const glm::vec2 &value) {
    glUniform2fv(glGetUniformLocation(programId, name.c_str()), 1, &value[0]);
}

static void setVec2(GLuint programId, const std::string &name, float x, float y) {
    glUniform2f(glGetUniformLocation(programId, name.c_str()), x, y);
}

static void setVec3(GLuint programId, const std::string &name, const glm::vec3 &value) {
    glUniform3fv(glGetUniformLocation(programId, name.c_str()), 1, &value[0]);
}

static void setVec3(GLuint programId, const std::string &name, float x, float y, float z) {
    glUniform3f(glGetUniformLocation(programId, name.c_str()), x, y, z);
}

static void setVec4(GLuint programId, const std::string &name, const glm::vec4 &value) {
    glUniform4fv(glGetUniformLocation(programId, name.c_str()), 1, &value[0]);
}

static void setVec4(GLuint programId, const std::string &name, float x, float y, float z, float w) {
    glUniform4f(glGetUniformLocation(programId, name.c_str()), x, y, z, w);
}

static void setMat2(GLuint programId, const std::string &name, const glm::mat2 &mat) {
    glUniformMatrix2fv(glGetUniformLocation(programId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

static void setMat3(GLuint programId, const std::string &name, const glm::mat3 &mat) {
    glUniformMatrix3fv(glGetUniformLocation(programId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

static void setMat4(GLuint programId, const std::string &name, const glm::mat4 &mat) {
    glUniformMatrix4fv(glGetUniformLocation(programId, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

static glm::vec3 texCoordToVertexCoord(glm::vec2 texCoord) {
    return glm::vec3(2 * texCoord.x - 1, 1 - 2 * texCoord.y, 0);
}

static const char *vShaderStr =
        "#version 300 es\n"
        "layout(location = 0) in vec4 a_position;\n"
        "layout(location = 1) in vec2 a_texCoord;\n"
        "uniform mat4 u_MVPMatrix;\n"
        "out vec2 v_texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = u_MVPMatrix * a_position;\n"
        "    v_texCoord = a_texCoord;\n"
        "}";

static const char *fShaderStr =
        "#version 300 es\n"
        "precision highp float;\n"
        "in vec2 v_texCoord;\n"
        "layout(location = 0) out vec4 outColor;\n"
        "uniform sampler2D s_texture0;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    outColor = texture(s_texture0, v_texCoord);\n"
        "}";

static GLfloat verticesCoords[] = {
        -1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
};

static GLfloat textureCoords[] = {
        0.0f,  0.0f,
        0.0f,  1.0f,
        1.0f,  1.0f,
        1.0f,  0.0f
};

static GLfloat mirror_textureCoords[] = {
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

media::fbo_paint::fbo_paint()
:cvs_width(0), cvs_height(0), cvs_ratio(0),
matrix(), program(GL_NONE), fbo_program(GL_NONE), textures(), vao(GL_NONE), vbo(),
src_fbo(GL_NONE), src_fbo_texture(GL_NONE), dst_fbo(GL_NONE), dst_fbo_texture(GL_NONE), frame_index(0) {
    log_d("created.");
}

media::fbo_paint::~fbo_paint() {
    log_d("release.");
}

void media::fbo_paint::set_canvas_size(int32_t width, int32_t height) {
    if (height == 0) {
        return;
    }

    cvs_width = width;
    cvs_height = height;
    cvs_ratio = (float)width/(float)height;
    glViewport(0, 0, cvs_width, cvs_height);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    log_d("canvas size: %d,%d %0.4f", cvs_width, cvs_height, cvs_ratio);
    frame_index = 0;

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
    if (!program || !fbo_program) {
        log_e("create program fail.");
        return;
    }

    glGenTextures(TEXTURE_NUM, textures);
    for (int i = 0; i < TEXTURE_NUM ; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, GL_NONE);
    }

    // Generate VBO Ids and load the VBOs with data
    glGenBuffers(3, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Generate VAO Id
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

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

    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if(program == GL_NONE || data == nullptr) return;

    glBindTexture(GL_TEXTURE_2D, GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);

    if(src_fbo == GL_NONE) {
        log_e("GLCameraRender::OnDrawFrame CreateFrameBufferObj fail");
        return;
    }

//    frame_index++;

//    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(verticesCoords), verticesCoords, GL_STATIC_DRAW);

//    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords),
//            mirror ? mirror_textureCoords : textureCoords, GL_STATIC_DRAW);

//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 渲染到 FBO
    glBindFramebuffer(GL_FRAMEBUFFER, src_fbo);
    glViewport(0, 0, cvs_width, cvs_height);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(fbo_program);

    glBindTexture(GL_TEXTURE_2D, src_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, dst_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    glBindVertexArray(vao);
//    update_matrix(&transform_matrix);
    update_matrix(0, 0, 1.0, 1.0);
    setMat4(fbo_program, "u_MVPMatrix", matrix);
    for (int i = 0; i < TEXTURE_NUM; ++i) {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        char samplerName[64] = {0};
        sprintf(samplerName, "s_texture%d", i);
        setInt(fbo_program, samplerName, i);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    // 再绘制一次，把方向倒过来
    glBindFramebuffer(GL_FRAMEBUFFER, dst_fbo);
    glViewport(0, 0, width, height);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (program);
    glBindVertexArray(vao);

    update_matrix(0, 0, 1.0, 1.0);
    setMat4(program, "u_MVPMatrix", matrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, src_fbo_texture);
    setInt(program, "s_texture0", 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    // 获得图像
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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 渲染到屏幕
    glViewport(0, 0, cvs_width, cvs_height);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    update_matrix(0, 0, 1.0, 1.0);
    setMat4(program, "u_MVPMatrix", matrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, dst_fbo_texture);
    setInt(program, "s_texture0", 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
}

void media::fbo_paint::update_matrix(int32_t angleX, int32_t angleY, float scaleX, float scaleY) {
    angleX = angleX % 360;
    angleY = angleY % 360;

    auto radiansX = static_cast<float>(MATH_PI / 180.0f * angleX);
    auto radiansY = static_cast<float>(MATH_PI / 180.0f * angleY);
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
