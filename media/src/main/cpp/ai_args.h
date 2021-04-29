//
// Created by Scliang on 3/8/21.
//

#ifndef STARS_AI_ARGS_H
#define STARS_AI_ARGS_H

namespace media {

typedef struct image_args {
    uint32_t width, height, channels, fps, frame_size;
    void update_frame_size() { frame_size = width*height*channels; }
} image_args;

typedef struct audio_args {
    uint32_t channels, sample_rate, frame_size;
} audio_args;

} //namespace media

#endif //STARS_AI_ARGS_H
