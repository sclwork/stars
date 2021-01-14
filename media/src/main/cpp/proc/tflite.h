//
// Created by scliang on 1/14/21.
//

#ifndef STARS_TFLITE_H
#define STARS_TFLITE_H

#include <tf/c_api.h>

namespace media {

class tflite {
public:
    tflite();
    ~tflite();

private:
    tflite(tflite&&) = delete;
    tflite(const tflite&) = delete;
    tflite& operator=(tflite&&) = delete;
    tflite& operator=(const tflite&) = delete;
};

} //namespace media

#endif //STARS_TFLITE_H
