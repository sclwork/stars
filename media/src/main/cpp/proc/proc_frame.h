//
// Created by Scliang on 3/22/21.
//

#ifndef STARS_PROC_FRAME_H
#define STARS_PROC_FRAME_H

#include "image_frame.h"
#include "audio_frame.h"

namespace media {

class frame {
public:
    frame():image(nullptr), audio(nullptr) {}
    frame(std::shared_ptr<image_frame> &&img, std::shared_ptr<audio_frame> &&aud)
            :image(img==nullptr?nullptr:std::make_shared<image_frame>(*img)),
             audio(aud==nullptr?nullptr:std::make_shared<audio_frame>(*aud)) {}
    ~frame() = default;

public:
    std::shared_ptr<image_frame> image;
    std::shared_ptr<audio_frame> audio;
};

} //namespace media

#endif //STARS_PROC_FRAME_H
