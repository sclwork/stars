//
// Created by scliang on 1/7/21.
//

#ifndef STARS_RENDERER_H
#define STARS_RENDERER_H

#include <cstdint>
#include "proc/image_frame.h"

namespace media {

class renderer {
public:
    virtual ~renderer() {}

public:
    virtual void surface_created() = 0;
    virtual void surface_destroyed() = 0;
    virtual void surface_changed(int32_t w, int32_t h) = 0;
    virtual void draw_frame(const std::shared_ptr<image_frame> &frame) = 0;
};

} //namespace media

#endif //STARS_RENDERER_H
