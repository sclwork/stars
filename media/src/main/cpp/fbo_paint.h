//
// Created by Scliang on 3/30/21.
//

#ifndef STARS_FBO_PAINT_H
#define STARS_FBO_PAINT_H

#include "gl_paint.h"

namespace media {

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
    std::shared_ptr<image_frame> draw(const std::shared_ptr<image_frame> &frame) override;

private:
    fbo_paint(fbo_paint&&) = delete;
    fbo_paint(const fbo_paint&) = delete;
    fbo_paint& operator=(fbo_paint&&) = delete;
    fbo_paint& operator=(const fbo_paint&) = delete;

private:
    int32_t   cvs_width;
    int32_t   cvs_height;
    float     cvs_ratio;
};

} //namespace media

#endif //STARS_FBO_PAINT_H
