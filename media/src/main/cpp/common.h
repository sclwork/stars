//
// Created by scliang on 1/5/21.
//

#ifndef STARS_COMMON_H
#define STARS_COMMON_H

#include <time.h>
#include <string>
#include "log.h"
#include "video/collect/recorder.h"
#include "video/play/renderer.h"
#include "video/proc/mnn.h"

#define LOG_DRAW_TIME 0

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
int32_t renderer_surface_created();
void renderer_surface_destroyed();
void renderer_surface_changed(int32_t w, int32_t h);
void renderer_draw_frame();
void renderer_select_camera(int camera);

// renderer_draw_frame tmp args
struct frame_args {
#if LOG_ABLE && LOG_DRAW_TIME
    struct timespec t;
    int32_t d_ns;
    long ns;
#endif
    int32_t frame_width;
    int32_t frame_height;
    uint32_t *frame_cache;
    std::vector<cv::Rect> faces;
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
    int32_t renderer_surface_created();
    void renderer_surface_destroyed();
    void renderer_surface_changed(int32_t w, int32_t h);
    void renderer_draw_frame();
    void renderer_select_camera(int camera);

private:
    common(common&&) = delete;
    common(const common&) = delete;
    common& operator=(common&&) = delete;
    common& operator=(const common&) = delete;

private:
    struct frame_args frame_args;
    //////////////////////////
    std::string cas_path;
    //////////////////////////
    renderer   *renderer;
    recorder   *recorder;
    //////////////////////////
    std::shared_ptr<mnn> mnn;
};

} //namespace media

#endif //STARS_COMMON_H
