//
// Created by scliang on 1/6/21.
//

#ifndef STARS_PAINT_H
#define STARS_PAINT_H

#include <cstdint>
#include "image_frame.h"

namespace media {

class paint {
public:
    virtual ~paint() {}

public:
    /**
     * setup canvas size
     * @param width canvas width
     * @param height canvas height
     */
    virtual void set_canvas_size(int32_t width, int32_t height) = 0;
    /**
     * draw image frame
     * @param frame image frame
     * @return new image_frame need delete
     */
    virtual void draw(const image_frame &frame, image_frame *of= nullptr) = 0;
};

} //namespace media

#endif //STARS_PAINT_H
