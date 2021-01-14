//
// Created by scliang on 1/7/21.
//

#ifndef STARS_IMAGE_RENDERER_H
#define STARS_IMAGE_RENDERER_H

#include "paint.h"
#include "renderer.h"

namespace media {

class image_renderer: public renderer {
public:
    image_renderer();
    ~image_renderer();

public:
    void surface_created() override;
    void surface_destroyed() override;
    void surface_changed(int32_t w, int32_t h) override;
    void draw_frame(std::shared_ptr<image_frame> &frame) override;
    void get_size(int32_t *ow, int32_t *oh) override;

private:
    image_renderer(image_renderer&&) = delete;
    image_renderer(const image_renderer&) = delete;
    image_renderer& operator=(image_renderer&&) = delete;
    image_renderer& operator=(const image_renderer&) = delete;

private:
    int32_t width;
    int32_t height;
    paint  *paint;
};

} //namespace media

#endif //STARS_IMAGE_RENDERER_H
