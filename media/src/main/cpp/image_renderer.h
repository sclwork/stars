//
// Created by scliang on 1/7/21.
//

#ifndef STARS_IMAGE_RENDERER_H
#define STARS_IMAGE_RENDERER_H

#include "paint.h"
#include "utils.h"
#include "concurrent_queue.h"

namespace media {

class image_renderer {
public:
    image_renderer(std::string &froot,
                   moodycamel::ConcurrentQueue<image_frame> &iQ,
                   bool (*cvrecording)());
    ~image_renderer();

public:
    /**
     * run in renderer thread.
     */
    void surface_created(const std::string &effect);
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
    void updt_frame(image_frame &&frm);
    /**
     * run in renderer thread.
     * read frm from frameQ and draw.
     */
    void draw_frame();
    /**
     * clear draw frame
     */
    void clear_frame();

public:
    /**
     * update effect paint
     */
    void updt_paint(const std::string &name);

private:
    paint *create_paint(const std::string &name);

private:
    image_renderer(image_renderer&&) = delete;
    image_renderer(const image_renderer&) = delete;
    image_renderer& operator=(image_renderer&&) = delete;
    image_renderer& operator=(const image_renderer&) = delete;

private:
    std::string &file_root;
    int32_t width;
    int32_t height;
    paint  *paint;
    moodycamel::ConcurrentQueue<image_frame> &eiQ;
    moodycamel::ConcurrentQueue<image_frame> drawQ;
    bool (*check_video_recording)();
};

} //namespace media

#endif //STARS_IMAGE_RENDERER_H
