//
// Created by Scliang on 4/21/21.
//

#ifndef STARS_DISTORTEDTV_PAINT_H
#define STARS_DISTORTEDTV_PAINT_H

#include "fbo_paint.h"
#include "proc.h"

namespace media {

class distortedtv_paint : public fbo_paint {
public:
    distortedtv_paint(std::string &froot, bool grey = true);
    ~distortedtv_paint();

private:
    distortedtv_paint(distortedtv_paint&&) = delete;
    distortedtv_paint(const distortedtv_paint&) = delete;
    distortedtv_paint& operator=(distortedtv_paint&&) = delete;
    distortedtv_paint& operator=(const distortedtv_paint&) = delete;

private:
    std::string gen_effect_frag_shader_str() override;
    void on_pre_tex_image(int32_t width, int32_t height, uint32_t *data) override;
    void on_setup_program_args(GLuint prog, const image_frame &frame) override;

private:
    bool    use_grey;
    int32_t f_index;
};

} //namespace media

#endif //STARS_DISTORTEDTV_PAINT_H
