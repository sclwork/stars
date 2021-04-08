//
// Created by Scliang on 3/30/21.
//

#ifndef STARS_FBO_PAINT_H
#define STARS_FBO_PAINT_H

#include <mutex>
#include "gl_paint.h"

namespace media {

struct TransformMatrix {
    int32_t degree;
    int32_t mirror;
    float translateX;
    float translateY;
    float scaleX;
    float scaleY;
    int32_t angleX;
    int32_t angleY;

    TransformMatrix():
            translateX(0),
            translateY(0),
            scaleX(1.0),
            scaleY(1.0),
            degree(0),
            mirror(0),
            angleX(0),
            angleY(0) {

    }

    void Reset() {
        translateX = 0;
        translateY = 0;
        scaleX = 1.0;
        scaleY = 1.0;
        degree = 0;
        mirror = 0;

    }
};

class fbo_paint : public gl_paint {
public:
    fbo_paint();
    ~fbo_paint();

public:
    /**
     * setup canvas size
     * @param width canvas width
     * @param height canvas height
     */
    void set_canvas_size(int32_t width, int32_t height) override;
    /**
     * draw image frame
     * @param frame image frame
     */
    void draw(const image_frame &frame, image_frame &of) override;

private:
    fbo_paint(fbo_paint&&) = delete;
    fbo_paint(const fbo_paint&) = delete;
    fbo_paint& operator=(fbo_paint&&) = delete;
    fbo_paint& operator=(const fbo_paint&) = delete;

private:
    void update_matrix(int32_t angleX, int32_t angleY, float scaleX, float scaleY);

private:
    int32_t   cvs_width;
    int32_t   cvs_height;
    float     cvs_ratio;
    /////////////////////////
    glm::mat4  matrix;
    /////////////////////////
    GLuint program;
    GLuint fbo_program;
    GLuint texture;
    GLuint vao;
    GLuint vbo[3];
    GLuint src_fbo;
    GLuint src_fbo_texture;
    GLuint dst_fbo;
    GLuint dst_fbo_texture;
    /////////////////////////
    int32_t frame_index;
};

} //namespace media

#endif //STARS_FBO_PAINT_H
