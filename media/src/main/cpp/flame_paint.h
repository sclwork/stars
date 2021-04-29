//
// Created by Scliang on 4/16/21.
//

#ifndef STARS_FLAME_PAINT_H
#define STARS_FLAME_PAINT_H

#include "fbo_paint.h"
#include "utils.h"

namespace media {

class flame_paint : public fbo_paint {
public:
    flame_paint(std::string &froot);
    ~flame_paint();

private:
    flame_paint(flame_paint&&) = delete;
    flame_paint(const flame_paint&) = delete;
    flame_paint& operator=(flame_paint&&) = delete;
    flame_paint& operator=(const flame_paint&) = delete;

private:
    std::string gen_effect_frag_shader_str() override;
    void on_setup_program_args(GLuint prog, const image_frame &frame) override;

private:
    int32_t f_index;
};

} //namespace media

#endif //STARS_FLAME_PAINT_H
