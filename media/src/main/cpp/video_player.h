//
// Created by Scliang on 4/28/21.
//

#ifndef STARS_VIDEO_PLAYER_H
#define STARS_VIDEO_PLAYER_H

#include <memory>
#include <stdint.h>
#include "utils.h"
#include "image_frame.h"
#include "audio_frame.h"
#include "ai_args.h"
#include "concurrent_queue.h"
#include "video_decoder.hpp"

namespace media {

class video_player {
public:
    video_player();
    ~video_player();

public:
    /**
      * start video play from to mp4 file
      * @param name mp4 file path / rtmp url
      */
    void start_play(int32_t w, int32_t h, std::string &&name,
                    void (*callback)(image_frame&&));
    /**
     * stop video play
     */
    void stop_play();
    /**
     * check play is running
     * @return true: playing
     */
    bool playing() const { return plying; }

private:
    video_player(video_player&&) = delete;
    video_player(const video_player&) = delete;
    video_player& operator=(video_player&&) = delete;
    video_player& operator=(const video_player&) = delete;

private:
    std::atomic_bool plying;
    //////////////////////////////////////
    std::shared_ptr<video_decoder> decoder;
};

} //namespace media

#endif //STARS_VIDEO_PLAYER_H
