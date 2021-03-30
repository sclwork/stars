//
// Created by Scliang on 3/30/21.
//

#ifndef STARS_GL_PAINT_H
#define STARS_GL_PAINT_H

#include <stdint.h>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/ext.hpp>
#include <GLES3/gl3.h>
#include "paint.h"

namespace media {

const float MATH_PI = 3.1415926535897932384626433832802f;

class gl_paint : public paint {
public:
    virtual ~gl_paint() {}

public:
    static GLuint load_shader(GLenum shaderType, const char *pSource);
    static GLuint create_program(const char *pVertexShaderSource, const char *pFragShaderSource,
                                 GLuint &vertexShaderHandle, GLuint &fragShaderHandle);
};

} //namespace media

#endif //STARS_GL_PAINT_H
