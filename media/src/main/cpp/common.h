//
// Created by scliang on 1/5/21.
//

#ifndef STARS_COMMON_H
#define STARS_COMMON_H

#include <map>
#include <time.h>
#include <string>
#include "log.h"
#include "proc/mnn.h"
#include "proc/ffmpeg.h"
#include "image/collect/image_recorder.h"
#include "audio/collect/audio_recorder.h"
#include "image/play/renderer.h"

namespace media {

/**
 * media common object/res ...
 */
class common {
public:
    common(std::string &&cascade, std::string &&mnn);
    ~common();

/*
 * run in renderer thread.
 */
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
    renderer *renderer;
    std::shared_ptr<image_frame> shw_frame;
};

} //namespace media

#endif //STARS_COMMON_H
