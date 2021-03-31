//
// Created by scliang on 1/14/21.
//

#ifndef STARS_FFMPEG_MP4_H
#define STARS_FFMPEG_MP4_H

#include "ffmpeg_h264.h"

namespace media {

class ffmpeg_mp4 : public ffmpeg_h264 {
public:
    ffmpeg_mp4(std::string &&n, image_args &&img, audio_args &&aud);
    ~ffmpeg_mp4();

public:
    void init() override;

private:
    ffmpeg_mp4(ffmpeg_mp4&&) = delete;
    ffmpeg_mp4(const ffmpeg_mp4&) = delete;
    ffmpeg_mp4& operator=(ffmpeg_mp4&&) = delete;
    ffmpeg_mp4& operator=(const ffmpeg_mp4&) = delete;

private:
    std::string _tmp;
    std::string name;
    std::string f_rgb_name;
    std::string f_264_name;
    std::string f_pcm_name;
    std::string f_aac_name;
};

} //namespace media

#endif //STARS_FFMPEG_MP4_H
