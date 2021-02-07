//
// Created by Scliang on 2/8/21.
//

#ifndef STARS_JNI_BRIDGE_H
#define STARS_JNI_BRIDGE_H

#include <stdint.h>

namespace media {

/*
 * [call from jni] media main thread loop start/exit
 * create/release global thread objects[common/queue_main]
 */
void loop_start(const char *cascade, const char *mnn);
void loop_exit();

/*
 * [call from jni] renderer thread
 * call to global common
 */
void renderer_init();
void renderer_release();
void renderer_surface_created();
void renderer_surface_destroyed();
void renderer_surface_changed(int32_t w, int32_t h);
void renderer_draw_frame();

} //namespace media

#endif //STARS_JNI_BRIDGE_H
