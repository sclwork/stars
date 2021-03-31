//
// Created by scliang on 1/6/21.
//

#include "jni_log.h"
#include "camera_paint.h"

#define log_d(...)  LOG_D("Media-Native:camera_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:camera_paint", __VA_ARGS__)

namespace media {
} //namespace media

media::camera_paint::camera_paint()
:texture(GL_NONE), vertex_shader(GL_NONE), fragment_shader(GL_NONE), program(GL_NONE),
sampler_location(0), sampler_matrix(0), cvs_width(0), cvs_height(0), cvs_ratio(0), matrix(glm::mat4{}) {
    glGenTextures  (1, &texture);
    glBindTexture  (GL_TEXTURE_2D, texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture  (GL_TEXTURE_2D, GL_NONE);

    char vShaderStr[] =
            "#version 300 es                                     \n"
            "layout(location = 0) in vec4 a_position;            \n"
            "layout(location = 1) in vec2 a_texCoord;            \n"
            "uniform mat4 u_Matrix;                              \n"
            "out vec2 v_texCoord;                                \n"
            "void main()                                         \n"
            "{                                                   \n"
            "   gl_Position = u_Matrix * a_position;             \n"
            "   v_texCoord = a_texCoord;                         \n"
            "}                                                   \n";

    char fShaderStr[] =
            "#version 300 es                                     \n"
            "#extension GL_OES_EGL_image_external_essl3 : require\n"
            "precision mediump float;                            \n"
            "in vec2 v_texCoord;                                 \n"
            "layout(location = 0) out vec4 outColor;             \n"
            "uniform sampler2D sp_location;                      \n"
            "void main()                                         \n"
            "{                                                   \n"
            "  outColor = texture(sp_location, v_texCoord);      \n"
            "}                                                   \n";

    program = create_program(vShaderStr, fShaderStr, vertex_shader, fragment_shader);
    if (program) {
        sampler_location = glGetUniformLocation(program, "sp_location");
        sampler_matrix = glGetUniformLocation(program, "u_Matrix");
    }

    GLfloat vcs[] = {  -1.0f,  1.0f, 0.0f,
                       -1.0f, -1.0f, 0.0f,
                        1.0f, -1.0f, 0.0f,
                        1.0f,  1.0f, 0.0f,  };
    memcpy(vertices_coords, vcs, sizeof(GLfloat) * 12);

    GLfloat tcs[] = {  0.0f,  0.0f,
                       0.0f,  1.0f,
                       1.0f,  1.0f,
                       1.0f,  0.0f,  };
    memcpy(texture_coords, tcs, sizeof(GLfloat) * 8);

    GLfloat tcs_mirror[] = {  1.0f,  0.0f,
                              1.0f,  1.0f,
                              0.0f,  1.0f,
                              0.0f,  0.0f,  };
    memcpy(texture_coords_mirror, tcs_mirror, sizeof(GLfloat) * 8);

    GLushort is[6] = {  0, 1, 2,
                        0, 2, 3,  };
    memcpy(indices, is, sizeof(GLushort) * 6);

    log_d("created.");
}

media::camera_paint::~camera_paint() {
    glDeleteTextures(1, &texture);
    texture = GL_NONE;
    glDeleteProgram(program);
    program = GL_NONE;

    log_d("release.");
}

void media::camera_paint::set_canvas_size(int32_t width, int32_t height) {
    if (height == 0) {
        return;
    }

    cvs_width = width;
    cvs_height = height;
    cvs_ratio = (float)width/(float)height;
    glViewport(0, 0, cvs_width, cvs_height);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    log_d("canvas size: %d,%d %0.4f", cvs_width, cvs_height, cvs_ratio);
}

std::shared_ptr<media::image_frame> media::camera_paint::draw(const std::shared_ptr<image_frame> &frame) {
    if (frame == nullptr) {
        return std::shared_ptr<image_frame>(nullptr);
    }

    bool mirror = frame->use_mirror();
    int32_t width, height; uint32_t *data;
    frame->get(&width, &height, &data);

    if (data == nullptr || program == GL_NONE || texture == GL_NONE) {
        return std::shared_ptr<image_frame>(nullptr);
    }

    float img_r = (float)width/(float)height;
    update_matrix(0, 0, cvs_ratio>img_r?cvs_ratio/img_r:img_r/cvs_ratio);
    glClear(GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    //upload RGBA image data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, GL_NONE);

    // Use the program object
    glUseProgram(program);

    // Load the vertex position
    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof (GLfloat), vertices_coords);
    // Load the texture coordinate
    glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat),
                           mirror ? texture_coords_mirror : texture_coords);

    glEnableVertexAttribArray (0);
    glEnableVertexAttribArray (1);

    // Bind the RGBA map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Set the RGBA map sampler to texture unit to 0
    glUniform1i(sampler_location, 0);
    glUniformMatrix4fv(sampler_matrix, 1, GL_FALSE, &(matrix[0][0]));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    return std::make_shared<image_frame>(*frame);
}

void media::camera_paint::update_matrix(int32_t angleX, int32_t angleY, float ratio) {
    angleX = angleX % 360;
    angleY = angleY % 360;

    auto radiansX = MATH_PI / 180.0f * angleX;
    auto radiansY = MATH_PI / 180.0f * angleY;

    // Projection matrix
    glm::mat4 Projection = glm::ortho(-ratio, ratio, -1.0f, 1.0f, 0.0f, 100.0f);

    // View matrix
    glm::mat4 View = glm::lookAt(
            glm::vec3(0, 0, 4), // Camera is at (0,0,1), in World Space
            glm::vec3(0, 0, 0), // and looks at the origin
            glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::scale(Model, glm::vec3(1.0f, 1.0f, 1.0f));
    Model = glm::rotate(Model, radiansX, glm::vec3(1.0f, 0.0f, 0.0f));
    Model = glm::rotate(Model, radiansY, glm::vec3(0.0f, 1.0f, 0.0f));
    Model = glm::translate(Model, glm::vec3(0.0f, 0.0f, 0.0f));

    matrix = Projection * View * Model;
}
