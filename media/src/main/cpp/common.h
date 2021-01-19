//
// Created by scliang on 1/5/21.
//

#ifndef STARS_COMMON_H
#define STARS_COMMON_H

#include <time.h>
#include <string>
#include "log.h"
#include "proc/mnn.h"
#include "proc/ffmpeg.h"
#include "video/collect/image_recorder.h"
#include "audio/collect/audio_recorder.h"
#include "video/play/renderer.h"

#define LOG_DRAW_TIME 1
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

// renderer_draw_frame tmp args
struct frame_args {
#if LOG_ABLE && LOG_DRAW_TIME
    struct timespec t;
    int32_t d_ns;
    long ns;
    int32_t fps_sum;
    int32_t fps_count;
    std::string fps;
#endif
    std::vector<cv::Rect> faces;
};

enum class RECORD_STATE {
    NONE,
    RECORDING,
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
    void renderer_record_start(std::string &&name);
    void renderer_record_stop();
    bool renderer_record_running();

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
    renderer       *renderer;
    image_recorder *img_recorder;
    audio_recorder *aud_recorder;
    //////////////////////////
#if IMPORT_TFLITE
    std::shared_ptr<tflite>   tflite;
#endif
    std::shared_ptr<mnn>      mnn;
    std::shared_ptr<ffmpeg>   ffmpeg;
    //////////////////////////////////////
    std::atomic<RECORD_STATE> record_state;
};

} //namespace media

#endif //STARS_COMMON_H
