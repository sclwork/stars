//
// Created by Scliang on 2/7/21.
//

#ifndef STARS_VIDEO_RECORDER_H
#define STARS_VIDEO_RECORDER_H

#include <memory>
#include <stdint.h>
#include "proc/image_frame.h"
#include "image/collect/image_recorder.h"

namespace media {

class video_recorder {
public:
    video_recorder();
    ~video_recorder();

public:
    /**
     * start video preview
     * @param callback callback preview image frame data
     * @param w requested width
     * @param h requested height
     * @param camera camera index [0, camera::camera_count)
     */
     void start_preview(void (*callback)(std::shared_ptr<image_frame>&&),
                        int32_t w, int32_t h,
                        int32_t camera = 0);
     /**
      * stop video preview
      */
     void stop_preview();

private:
    video_recorder(video_recorder&&) = delete;
    video_recorder(const video_recorder&) = delete;
    video_recorder& operator=(video_recorder&&) = delete;
    video_recorder& operator=(const video_recorder&) = delete;
};

} //namespace media

#endif //STARS_VIDEO_RECORDER_H
