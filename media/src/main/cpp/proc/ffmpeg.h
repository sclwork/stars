//
// Created by scliang on 1/14/21.
//

#ifndef STARS_FFMPEG_H
#define STARS_FFMPEG_H

#include <memory>
#include "image_frame.h"
#include "audio_frame.h"

namespace media {

class ffmpeg {
public:
    ffmpeg();
    ~ffmpeg();

public:
    void video_encode_start(std::string &name);
    void video_encode_stop();
    void video_encode_frame(std::shared_ptr<image_frame> &&img_frame,
                            std::shared_ptr<audio_frame> &&aud_frame);

private:
    ffmpeg(ffmpeg&&) = delete;
    ffmpeg(const ffmpeg&) = delete;
    ffmpeg& operator=(ffmpeg&&) = delete;
    ffmpeg& operator=(const ffmpeg&) = delete;
};

} //namespace media

#endif //STARS_FFMPEG_H
