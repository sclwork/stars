//
// Created by Scliang on 4/14/21.
//

#ifndef STARS_RIPPLE_PAINT_H
#define STARS_RIPPLE_PAINT_H

#include "fbo_paint.h"
#include "proc.h"

namespace media {

class ripple_paint : public fbo_paint {
public:
    ripple_paint();
    ~ripple_paint();

private:
    ripple_paint(ripple_paint&&) = delete;
    ripple_paint(const ripple_paint&) = delete;
    ripple_paint& operator=(ripple_paint&&) = delete;
    ripple_paint& operator=(const ripple_paint&) = delete;

private:
    const char *gen_effect_frag_shader_str() override;
    void on_setup_program_args(GLuint prog, const image_frame &frame) override;

private:
    long    rfx[3];
    long    rfy[3];
    int32_t f_index;
    kalman  k_face_x;
    kalman  k_face_y;
    kalman  k_face_z;
    kalman  k_face_w;
};

} //namespace media

#endif //STARS_RIPPLE_PAINT_H
