//
// Created by scliang on 1/6/21.
//

#ifndef STARS_PAINT_H
#define STARS_PAINT_H

namespace recorder {

class paint {
public:
    virtual void set_canvas_size(int32_t width, int32_t height) = 0;
    virtual void draw(int32_t width, int32_t height, uint32_t *data) = 0;
};

} //namespace recorder

#endif //STARS_PAINT_H
