//
// Created by Scliang on 3/8/21.
//

#ifndef STARS_FFMPEG_RTMP_H
#define STARS_FFMPEG_RTMP_H

#include "ffmpeg_h264.h"

namespace media {

class ffmpeg_rtmp : public ffmpeg_h264 {
public:
    ffmpeg_rtmp(std::string &&f, std::string &&n, image_args &&img, audio_args &&aud);
    ~ffmpeg_rtmp();

public:
    void init() override;

private:
    ffmpeg_rtmp(ffmpeg_rtmp&&) = delete;
    ffmpeg_rtmp(const ffmpeg_rtmp&) = delete;
    ffmpeg_rtmp& operator=(ffmpeg_rtmp&&) = delete;
    ffmpeg_rtmp& operator=(const ffmpeg_rtmp&) = delete;

private:
    std::string file;
    std::string name;
};

} //namespace media

#endif //STARS_FFMPEG_RTMP_H
