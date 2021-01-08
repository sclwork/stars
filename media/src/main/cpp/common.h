//
// Created by scliang on 1/5/21.
//

#ifndef STARS_COMMON_H
#define STARS_COMMON_H

#include <string>
#include "log.h"
#include "video/collect/recorder.h"
#include "video/play/renderer.h"

#define DRAW_TEST_FRAME 1
#define MATH_PI         3.1415926535897932384626433832802

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

class frame {
public:
    frame() = default;
    ~frame() = default;

public:
    void draw_to_renderer(renderer *);

#if DRAW_TEST_FRAME
public:
    void setup_test_data(int32_t w, int32_t h);
#endif

private:
    frame(frame&&) = delete;
    frame(const frame&) = delete;
    frame& operator=(frame&&) = delete;
    frame& operator=(const frame&) = delete;

private:
    int32_t   width;
    int32_t   height;
    uint32_t *data;
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
    std::string cascade;
    std::string mnn;
    //////////////////////////
    recorder   *recorder;
    renderer   *renderer;
#if DRAW_TEST_FRAME
    //////////////////////////
    frame      *test_frame;
#endif
};

} //media media

#endif //STARS_COMMON_H
