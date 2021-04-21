//
// Created by Scliang on 3/22/21.
//

#ifndef STARS_OPENCV_H
#define STARS_OPENCV_H

#include <cstdint>
#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#include <image_frame.h>

namespace media {

class opencv {
public:
    opencv();
    ~opencv();

public:
    static uint32_t *load_image(const std::string &file, int32_t *ow, int32_t *oh);
    static void grey_data(int32_t width, int32_t height, uint32_t *data);
    static void grey_frame(const image_frame &frame);

private:
    opencv(opencv&&) = delete;
    opencv& operator=(opencv&&) = delete;
    opencv& operator=(const opencv&) = delete;
};

} //namespace media

#endif //STARS_OPENCV_H
