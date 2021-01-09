//
// Created by scliang on 1/8/21.
//

#ifndef STARS_RECORDER_H
#define STARS_RECORDER_H

namespace media {

class recorder {
public:
    virtual ~recorder() {}

public:
    virtual void update_size(int32_t w, int32_t h) = 0;
    virtual void update_frame() = 0;
};

} //media media

#endif //STARS_RECORDER_H
