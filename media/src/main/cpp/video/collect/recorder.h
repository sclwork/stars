//
// Created by scliang on 1/8/21.
//

#ifndef STARS_RECORDER_H
#define STARS_RECORDER_H

#include <cstdint>
#include "image_cache.h"

namespace media {

class recorder {
public:
    virtual ~recorder() {}

public:
    virtual int32_t camera_count() const = 0;
    virtual void select_camera(int camera) = 0;
    virtual void update_size(int32_t w, int32_t h) = 0;
    virtual std::shared_ptr<image_cache> update_frame() = 0;
};

} //namespace media

#endif //STARS_RECORDER_H
