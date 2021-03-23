//
// Created by scliang on 1/7/21.
//

#ifndef STARS_IMAGE_RENDERER_H
#define STARS_IMAGE_RENDERER_H

#include "paint.h"
#include "loop/concurrent_queue.h"

namespace media {

class image_renderer {
public:
    image_renderer();
    ~image_renderer();

public:
    /**
     * run in renderer thread.
     */
    void surface_created();
    /**
     * run in renderer thread.
     */
    void surface_destroyed();
    /**
     * run in renderer thread.
     */
    void surface_changed(int32_t w, int32_t h);

public:
    /**
     * run in caller thread.
     * append frm to frameQ.
     */
    void updt_frame(const std::shared_ptr<image_frame> &&frm);
    /**
     * run in renderer thread.
     * read frm from frameQ and draw.
     */
    void draw_frame();

private:
    image_renderer(image_renderer&&) = delete;
    image_renderer(const image_renderer&) = delete;
    image_renderer& operator=(image_renderer&&) = delete;
    image_renderer& operator=(const image_renderer&) = delete;

private:
    int32_t width;
    int32_t height;
    paint  *paint;
    std::shared_ptr<image_frame> frame;
    moodycamel::ConcurrentQueue<std::shared_ptr<image_frame>> frameQ;
};

} //namespace media

#endif //STARS_IMAGE_RENDERER_H
