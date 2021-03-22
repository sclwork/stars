//
// Created by Scliang on 3/22/21.
//

#ifndef STARS_OPENCV_H
#define STARS_OPENCV_H

#include <cstdint>
#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
#include <proc/image_frame.h>

namespace media {

class opencv {
public:
    opencv();
    ~opencv();

public:
    static void grey_frame(const std::shared_ptr<image_frame> &frame);

private:
    opencv(opencv&&) = delete;
    opencv& operator=(opencv&&) = delete;
    opencv& operator=(const opencv&) = delete;
};

} //namespace media

#endif //STARS_OPENCV_H
