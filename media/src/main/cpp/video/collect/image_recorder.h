//
// Created by scliang on 1/6/21.
//

#ifndef STARS_IMAGE_RECORDER_H
#define STARS_IMAGE_RECORDER_H

#include <string>
#include "recorder.h"
#include "camera.h"

namespace media {

class image_recorder: public recorder {
public:
    image_recorder(int32_t width, int32_t height);
    ~image_recorder();

public:
    void update_size(int32_t w, int32_t h);
    void update_frame();

private:
    image_recorder(image_recorder&&) = delete;
    image_recorder(const image_recorder&) = delete;
    image_recorder& operator=(image_recorder&&) = delete;
    image_recorder& operator=(const image_recorder&) = delete;

private:
    int32_t width;
    int32_t height;
    /////////////////////////////////////////
    std::vector<std::shared_ptr<camera>> cams;
    std::shared_ptr<camera>run_cam;
};

} //namespace media

#endif //STARS_IMAGE_RECORDER_H
