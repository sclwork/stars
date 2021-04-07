//
// Created by scliang on 1/6/21.
//

#ifndef STARS_CAMERA_PAINT_H
#define STARS_CAMERA_PAINT_H

#include "gl_paint.h"

namespace media {

class camera_paint: public gl_paint {
public:
    camera_paint();
    ~camera_paint();

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
    void update_matrix(int32_t angleX, int32_t angleY, float ratio);

private:
    camera_paint(camera_paint&&) = delete;
    camera_paint(const camera_paint&) = delete;
    camera_paint& operator=(camera_paint&&) = delete;
    camera_paint& operator=(const camera_paint&) = delete;

private:
    GLuint    texture;
    GLuint    program;
    GLint     sampler_location;
    GLint     sampler_matrix;
    int32_t   cvs_width;
    int32_t   cvs_height;
    float     cvs_ratio;
    glm::mat4 matrix;
    GLfloat   vertices_coords[12]{};
    GLfloat   texture_coords[8]{};
    GLfloat   texture_coords_mirror[8]{};
    GLushort  indices[6]{};
};

} //namespace media

#endif //STARS_CAMERA_PAINT_H
