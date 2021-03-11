//
// Created by scliang on 1/5/21.
//

#ifndef STARS_COMMON_H
#define STARS_COMMON_H

#include <map>
#include <time.h>
#include <string>
#include "jni_log.h"
#include "image/play/image_renderer.h"
#include "video/video_recorder.h"

namespace media {

/**
 * media common object/res ...
 */
class common {
public:
    common(std::string &&file_root, std::string &&cascade, std::string &&mnn);
    ~common();

public:
    /**
     * run in renderer thread.
     */
    void renderer_init();
    /**
     * run in renderer thread.
     */
    void renderer_release();
    /**
     * run in renderer thread.
     */
    void renderer_surface_created();
    /**
     * run in renderer thread.
     */
    void renderer_surface_destroyed();
    /**
     * run in renderer thread.
     */
    void renderer_surface_changed(int32_t w, int32_t h);
    /**
     * run in renderer thread.
     */
    void renderer_draw_frame();
    /**
     * start/stop/rtmp video record.
     */
    void video_record_start(std::string &&name);
    void video_record_stop();
    bool video_recording();

private:
    /*
     * run in caller thread.
     * copy from frm to renderer.
     */
    void renderer_updt_frame(const image_frame &frm);

private:
    common(common&&) = delete;
    common(const common&) = delete;
    common& operator=(common&&) = delete;
    common& operator=(const common&) = delete;

private:
    std::string file_root;
    std::string mnn_path;
    std::shared_ptr<image_renderer> renderer;
    std::shared_ptr<video_recorder> vid_rec;
};

} //namespace media

#endif //STARS_COMMON_H
