//
// Created by Scliang on 4/28/21.
//

#ifndef STARS_VIDEO_PLAYER_H
#define STARS_VIDEO_PLAYER_H

namespace media {

class video_player {
public:
    video_player();
    ~video_player();

private:
    video_player(video_player&&) = delete;
    video_player(const video_player&) = delete;
    video_player& operator=(video_player&&) = delete;
    video_player& operator=(const video_player&) = delete;
};

} //namespace media

#endif //STARS_VIDEO_PLAYER_H
