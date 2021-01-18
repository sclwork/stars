//
// Created by scliang on 1/6/21.
//

#ifndef STARS_IMAGE_RECORDER_H
#define STARS_IMAGE_RECORDER_H

#include <string>
#include "proc/image_frame.h"
#include "camera.h"

namespace media {

class image_recorder {
public:
    image_recorder();
    ~image_recorder();

public:
    int32_t camera_count() const;
    bool is_previewing() const;
    bool select_camera(int camera);
    void update_size(int32_t w, int32_t h);
    std::shared_ptr<image_frame> collect_frame();

private:
    image_recorder(image_recorder&&) = delete;
    image_recorder(const image_recorder&) = delete;
    image_recorder& operator=(image_recorder&&) = delete;
    image_recorder& operator=(const image_recorder&) = delete;

private:
    bool previewing;
    std::shared_ptr<image_frame> frame;
    /////////////////////////////////////////
    std::vector<std::shared_ptr<camera>> cams;
    std::shared_ptr<camera>run_cam;
};

} //namespace media

#endif //STARS_IMAGE_RECORDER_H
