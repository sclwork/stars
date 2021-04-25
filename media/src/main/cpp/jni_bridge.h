//
// Created by Scliang on 2/8/21.
//

#ifndef STARS_JNI_BRIDGE_H
#define STARS_JNI_BRIDGE_H

#include <string>
#include <stdint.h>

namespace media {

/**
 * [call from jni] media main thread loop start/exit
 * create/release global thread objects[common/queue_main]
 */
void loop_start(const std::string &file_root,
                const std::string &cascade,
                const std::string &mnn,
                void (*on_request_render)(int32_t));
void loop_exit();

/**
 * [call from jni] renderer thread
 * call to global common
 */
void renderer_init();
void renderer_release();
void renderer_surface_created();
void renderer_surface_destroyed();
void renderer_surface_changed(int32_t w, int32_t h);
void renderer_draw_frame();
void renderer_updt_paint(const std::string &name);

/**
 * mp4/rtmp video record
 */
void video_record_start(const std::string &name);
void video_record_stop();
bool video_recording();

/**
 * camera setup params
 */
void camera_select(int32_t cam);

} //namespace media

#endif //STARS_JNI_BRIDGE_H
