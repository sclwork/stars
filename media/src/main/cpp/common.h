//
// Created by scliang on 1/5/21.
//

#ifndef STARS_COMMON_H
#define STARS_COMMON_H

#include <string>
#include "log.h"
#include "video/collect/recorder.h"
#include "video/play/renderer.h"

namespace media {

/*
 * media main thread loop start/exit
 */
void loop_start(const char *cascade, const char *mnn);
void loop_exit();

/*
 * post runnable to media main thread loop
 */
void loop_post(void (*runnable)(void *ctx, void (*callback)(void *ctx)),
               void *ctx = nullptr,
               void (*callback)(void *ctx) = nullptr);

/*
 * renderer
 */
void renderer_init();
void renderer_release();
void renderer_surface_created();
void renderer_surface_destroyed();
void renderer_surface_changed(int32_t w, int32_t h);
void renderer_draw_frame();

// renderer_draw_frame tmp args
struct frame_args {
    int32_t frame_width;
    int32_t frame_height;
    uint32_t *frame_cache;
};

/**
 * media common object/res ...
 */
class common {
public:
    common(std::string &cascade, std::string &mnn);
    ~common();

public:
    void renderer_init();
    void renderer_release();
    void renderer_surface_created();
    void renderer_surface_destroyed();
    void renderer_surface_changed(int32_t w, int32_t h);
    void renderer_draw_frame();

private:
    common(common&&) = delete;
    common(const common&) = delete;
    common& operator=(common&&) = delete;
    common& operator=(const common&) = delete;

private:
    struct frame_args frame_args;
    //////////////////////////
    std::string cascade;
    std::string mnn;
    //////////////////////////
    recorder   *recorder;
    renderer   *renderer;
};

} //media media

#endif //STARS_COMMON_H
