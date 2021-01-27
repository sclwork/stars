//
// Created by scliang on 1/6/21.
//

#ifndef STARS_CAMERA_PAINT_H
#define STARS_CAMERA_PAINT_H

#include <stdint.h>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/ext.hpp>
#include <GLES3/gl3.h>
#include "paint.h"

namespace media {

class camera_paint: public paint {
public:
    camera_paint();
    ~camera_paint();

public:
    void set_canvas_size(int32_t width, int32_t height) override;
    void draw(const std::shared_ptr<image_frame> &frame) override;

private:
    void update_matrix(int32_t angleX, int32_t angleY, float ratio);
    static GLuint load_shader(GLenum shaderType, const char *pSource);
    static GLuint create_program(const char *pVertexShaderSource, const char *pFragShaderSource,
                          GLuint &vertexShaderHandle, GLuint &fragShaderHandle);

private:
    camera_paint(camera_paint&&) = delete;
    camera_paint(const camera_paint&) = delete;
    camera_paint& operator=(camera_paint&&) = delete;
    camera_paint& operator=(const camera_paint&) = delete;

private:
    GLuint    texture;
    GLuint    vertex_shader;
    GLuint    fragment_shader;
    GLuint    program;
    GLint     sampler_location;
    GLint     sampler_matrix;
    int32_t   cvs_width;
    int32_t   cvs_height;
    float     cvs_ratio;
    glm::mat4 matrix;
    GLfloat   vertices_coords[12]{};
    GLfloat   texture_coords[8]{};
    GLushort  indices[6]{};
};

} //namespace media

#endif //STARS_CAMERA_PAINT_H
