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
#include "video/collect/image_recorder.h"
#include "audio/collect/audio_recorder.h"
#include "video/play/renderer.h"

#define IMPORT_TFLITE 1

#if IMPORT_TFLITE
#include <proc/tflite.h>
#include <opencv2/opencv.hpp>
#endif

namespace media {

/*
 * media main thread loop start/exit
 */
void loop_start(const char *cascade, const char *mnn);
void loop_exit();

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
void renderer_record_start(const char *name);
void renderer_record_stop();
bool renderer_record_running();

class show_frame {
public:
    show_frame() = default;
    ~show_frame() = default;

public:
    void set_frame(std::shared_ptr<image_frame> &&frm);
    std::shared_ptr<image_frame> get_frame() const;
    std::mutex &get_mutex();

private:
    std::shared_ptr<image_frame> frame;
    mutable std::mutex           frame_mutex;
};

class mnns {
public:
    mnns(std::string &mnn_path);
    ~mnns() = default;

public:
    std::shared_ptr<mnn> get_mnn(std::__thread_id id);

private:
#ifndef USE_SINGLE_THREAD
    std::shared_ptr<mnn> mnn_arr[3];
    std::__thread_id thrd_ids[3];
#else
    std::shared_ptr<mnn> mnn;
#endif
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
    void renderer_select_camera(int32_t camera);
    void renderer_record_start(std::string &&name);
    void renderer_record_stop();
    bool renderer_record_running();

private:
    common(common&&) = delete;
    common(const common&) = delete;
    common& operator=(common&&) = delete;
    common& operator=(const common&) = delete;

private:
    std::string cas_path;
    //////////////////////////
    std::shared_ptr<renderer>       renderer;
    std::shared_ptr<image_recorder> img_recorder;
    std::shared_ptr<audio_recorder> aud_recorder;
    std::shared_ptr<show_frame>     shw_frame;
    //////////////////////////
#if LOG_ABLE && LOG_DRAW_TIME
    long draw_t;
#endif
#if IMPORT_TFLITE
    std::shared_ptr<tflite>   tflite;
#endif
    std::shared_ptr<mnns>     mnns;
    std::shared_ptr<ffmpeg>   ffmpeg;
};

} //namespace media

#endif //STARS_COMMON_H
