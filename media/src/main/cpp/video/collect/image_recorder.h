//
// Created by scliang on 1/6/21.
//

#ifndef STARS_IMAGE_RECORDER_H
#define STARS_IMAGE_RECORDER_H

#include <string>
#include "recorder.h"
#include "image_cache.h"
#include "camera.h"

namespace media {

// update_frame tmp args
struct cache_args {
    int32_t cache_width;
    int32_t cache_height;
    uint32_t *cache_cache;
    int32_t img_width;
    int32_t img_height;
    uint32_t *img_cache;
    int32_t wof;
    int32_t hof;
};

class image_recorder: public recorder {
public:
    image_recorder();
    ~image_recorder();

public:
    int32_t camera_count() const override;
    void select_camera(int camera) override;
    void update_size(int32_t w, int32_t h) override;
    std::shared_ptr<image_cache> update_frame() override;

private:
    image_recorder(image_recorder&&) = delete;
    image_recorder(const image_recorder&) = delete;
    image_recorder& operator=(image_recorder&&) = delete;
    image_recorder& operator=(const image_recorder&) = delete;

private:
    struct cache_args cache_args;
    std::shared_ptr<image_cache> cache;
    /////////////////////////////////////////
    std::vector<std::shared_ptr<camera>> cams;
    std::shared_ptr<camera>run_cam;
};

} //namespace media

#endif //STARS_IMAGE_RECORDER_H
