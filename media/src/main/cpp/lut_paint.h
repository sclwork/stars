//
// Created by Scliang on 4/13/21.
//

#ifndef STARS_LUT_PAINT_H
#define STARS_LUT_PAINT_H

#include "fbo_paint.h"

namespace media {

class lut_paint : public fbo_paint {
public:
    lut_paint(std::string &froot);
    ~lut_paint();

private:
    lut_paint(lut_paint&&) = delete;
    lut_paint(const lut_paint&) = delete;
    lut_paint& operator=(lut_paint&&) = delete;
    lut_paint& operator=(const lut_paint&) = delete;

private:
    std::string gen_effect_frag_shader_str() override;
    void on_canvas_size_changed(int32_t width, int32_t height) override;
    void on_setup_program_args(GLuint prog, const image_frame &frame) override;

private:
    GLuint       lut_texture;
    int32_t      lut_wid;
    int32_t      lut_hei;
    uint32_t    *lut_img;
};

} //namespace media

#endif //STARS_LUT_PAINT_H
