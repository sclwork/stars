//
// Created by Scliang on 2/7/21.
//

#ifndef STARS_VIDEO_RECORDER_H
#define STARS_VIDEO_RECORDER_H

namespace media {

class video_recorder {
public:
    video_recorder();
    ~video_recorder();

private:
    video_recorder(video_recorder&&) = delete;
    video_recorder(const video_recorder&) = delete;
    video_recorder& operator=(video_recorder&&) = delete;
    video_recorder& operator=(const video_recorder&) = delete;
};

} //namespace media

#endif //STARS_VIDEO_RECORDER_H
