//
// Created by Scliang on 2/7/21.
//

#ifndef STARS_VIDEO_RECORDER_H
#define STARS_VIDEO_RECORDER_H

#include <memory>
#include <stdint.h>
#include "proc.h"
#include "image_frame.h"
#include "audio_frame.h"
#include "ffmpeg_args.h"
#include "concurrent_queue.h"

#include "video_encoder.hpp"
#include "video_collector.hpp"

namespace media {

class video_recorder {
public:
    video_recorder(std::string &mnn_path,
                   moodycamel::ConcurrentQueue<image_frame> &iQ,
                   moodycamel::ConcurrentQueue<audio_frame> &aQ);
    ~video_recorder();

public:
    /**
     * start video preview
     * @param callback callback preview image frame data
     * @param w requested width
     * @param h requested height
     * @param camera camera index [0, camera::camera_count)
     */
     void start_preview(void (*callback)(image_frame&&),
                        int32_t w, int32_t h,
                        int32_t camera = 0);
     /**
      * stop video preview
      */
     void stop_preview();
     /**
      * start video record save to mp4 file
      * @param name mp4 file path / rtmp url
      */
     void start_record(std::string &&file_root, std::string &&name);
     /**
      * stop video record
      */
     void stop_record();
     /**
      * check record is running
      * @return true: recording
      */
     bool recording() const { return recing; }

private:
    video_recorder(video_recorder&&) = delete;
    video_recorder(const video_recorder&) = delete;
    video_recorder& operator=(video_recorder&&) = delete;
    video_recorder& operator=(const video_recorder&) = delete;

private:
    std::string mnn_path;
    std::atomic_bool recing;
    moodycamel::ConcurrentQueue<image_frame> &eiQ;
    moodycamel::ConcurrentQueue<audio_frame> &eaQ;
    //////////////////////////////////////
    std::shared_ptr<video_collector> collector;
    std::shared_ptr<video_encoder>   encoder;
};

} //namespace media

#endif //STARS_VIDEO_RECORDER_H
