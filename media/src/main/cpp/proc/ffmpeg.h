//
// Created by scliang on 1/14/21.
//

#ifndef STARS_FFMPEG_H
#define STARS_FFMPEG_H

#include <mutex>
#include <string>
#include "image_frame.h"
#include "audio_frame.h"

namespace media {

class ffmpeg {
public:
    ffmpeg();
    ~ffmpeg();

private:
    ffmpeg(ffmpeg&&) = delete;
    ffmpeg(const ffmpeg&) = delete;
    ffmpeg& operator=(ffmpeg&&) = delete;
    ffmpeg& operator=(const ffmpeg&) = delete;
};

} //namespace media

#endif //STARS_FFMPEG_H
