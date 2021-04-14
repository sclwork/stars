//
// Created by Scliang on 4/13/21.
//

#ifndef STARS_FACE_PAINT_H
#define STARS_FACE_PAINT_H

#include "fbo_paint.h"
#include "proc.h"

namespace media {

class face_paint : public fbo_paint {
public:
    face_paint();
    ~face_paint();

private:
    face_paint(face_paint&&) = delete;
    face_paint(const face_paint&) = delete;
    face_paint& operator=(face_paint&&) = delete;
    face_paint& operator=(const face_paint&) = delete;

private:
    const char *gen_effect_frag_shader_str() override;
    void on_setup_program_args(GLuint prog, const image_frame &frame) override;

private:
    kalman k_face_x;
    kalman k_face_y;
    kalman k_face_z;
    kalman k_face_w;
};

} //namespace media

#endif //STARS_FACE_PAINT_H
