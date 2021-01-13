//
// Created by scliang on 1/7/21.
//

#ifndef STARS_RENDERER_H
#define STARS_RENDERER_H

#include <cstdint>

namespace media {

class renderer {
public:
    virtual ~renderer() {}

public:
    virtual void surface_created() = 0;
    virtual void surface_destroyed() = 0;
    virtual void surface_changed(int32_t w, int32_t h) = 0;
    virtual void draw_frame(int32_t w, int32_t h, uint32_t *data) = 0;
    virtual void get_size(int32_t *ow, int32_t *oh) = 0;
};

} //namespace media

#endif //STARS_RENDERER_H
