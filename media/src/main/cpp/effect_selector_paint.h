//
// Created by Scliang on 4/22/21.
//

#ifndef STARS_EFFECT_SELECTOR_PAINT_H
#define STARS_EFFECT_SELECTOR_PAINT_H

#include "fbo_paint.h"
#include "proc.h"

namespace media {

enum effect_type {
    EFFECT_None,
    EFFECT_3Basic,
    EFFECT_EdgesBilateral,
    EFFECT_SinWave,
    EFFECT_Floyd,
    EFFECT_3Floyd,
    EFFECT_Distortedtv,
    EFFECT_DistortedtvBox,
    EFFECT_DistortedtvGlitch,
};

class effect_selector_paint : public fbo_paint {
public:
    effect_selector_paint(std::string &froot, effect_type type = EFFECT_None, bool grey = false);
    ~effect_selector_paint();

private:
    effect_selector_paint(effect_selector_paint&&) = delete;
    effect_selector_paint(const effect_selector_paint&) = delete;
    effect_selector_paint& operator=(effect_selector_paint&&) = delete;
    effect_selector_paint& operator=(const effect_selector_paint&) = delete;

private:
    std::string gen_effect_frag_shader_str() override;
    void on_pre_tex_image(int32_t width, int32_t height, uint32_t *data) override;
    void on_setup_program_args(GLuint prog, const image_frame &frame) override;

private:
    bool    use_grey;
    effect_type e_type;
    int32_t f_index;
};

} //namespace media

#endif //STARS_EFFECT_SELECTOR_PAINT_H
