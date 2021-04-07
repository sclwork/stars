//
// Created by Scliang on 3/30/21.
//

#include "jni_log.h"
#include "gl_paint.h"

#define log_d(...)  LOG_D("Media-Native:camera_paint", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:camera_paint", __VA_ARGS__)

namespace media {
} //namespace media

GLuint media::gl_paint::load_shader(GLenum shaderType, const char *pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, nullptr);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc((size_t)infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, nullptr, buf);
                    log_e("LoadShader Could not compile shader %d: %s", shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }

    return shader;
}

GLuint media::gl_paint::create_program(const char *pVertexShaderSource,
                                       const char *pFragShaderSource) {
    GLuint prog = 0;
    GLuint vertexShaderHandle = load_shader(GL_VERTEX_SHADER, pVertexShaderSource);
    if (!vertexShaderHandle) {
        return prog;
    }

    GLuint fragShaderHandle = load_shader(GL_FRAGMENT_SHADER, pFragShaderSource);
    if (!fragShaderHandle) {
        return prog;
    }

    prog = glCreateProgram();
    if (prog) {
        glAttachShader(prog, vertexShaderHandle);
        glAttachShader(prog, fragShaderHandle);
        glLinkProgram(prog);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(prog, GL_LINK_STATUS, &linkStatus);

        glDetachShader(prog, vertexShaderHandle);
        glDeleteShader(vertexShaderHandle);
        glDetachShader(prog, fragShaderHandle);
        glDeleteShader(fragShaderHandle);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*)malloc((size_t)bufLength);
                if (buf) {
                    glGetProgramInfoLog(prog, bufLength, nullptr, buf);
                    log_e("GLUtils::CreateProgram Could not link program: %s", buf);
                    free(buf);
                }
            }
            glDeleteProgram(prog);
            prog = 0;
        }
    }

    return prog;
}
