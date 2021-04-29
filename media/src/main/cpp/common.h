//
// Created by scliang on 1/5/21.
//

#ifndef STARS_COMMON_H
#define STARS_COMMON_H

#include <map>
#include <time.h>
#include <string>
#include "jni_log.h"
#include "image_renderer.h"
#include "video_recorder.h"
#include "video_player.h"

namespace media {

/**
 * media common object/res ...
 */
class common {
public:
    common(const std::string &file_root,
           const std::string &cascade,
           const std::string &mnn,
           void (*on_request_render)(int32_t));
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
     * update effect paint
     */
    void renderer_updt_paint(const std::string &name);
    /**
     * start/stop/rtmp video record.
     */
    void video_record_start(std::string &&name);
    void video_record_stop();
    bool video_recording();

    /**
     * camera setup params
     */
    void camera_select(int32_t cam);

    /**
     * start/stop/rtmp video play.
     */
    void video_play_start(std::string &&name);
    void video_play_stop();
    bool video_playing();

private:
    /*
     * run in caller thread.
     * copy from frm to renderer.
     */
    void renderer_updt_frame(image_frame &&frm);

private:
    common(common&&) = delete;
    common(const common&) = delete;
    common& operator=(common&&) = delete;
    common& operator=(const common&) = delete;

private:
    std::string file_root;
    std::string mnn_path;
    std::string effect_name;
    std::shared_ptr<image_renderer> renderer;
    std::shared_ptr<video_recorder> vid_rec;
    std::shared_ptr<video_player>   vid_ply;
    moodycamel::ConcurrentQueue<image_frame> eiQ;
    moodycamel::ConcurrentQueue<audio_frame> eaQ;
    void (*on_request_render_callback)(int32_t);
    /////////////////////////////////////////////
    int32_t surface_size[2];
};

} //namespace media

#endif //STARS_COMMON_H
