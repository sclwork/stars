//
// Created by scliang on 1/6/21.
//

#ifndef STARS_IMAGE_RECORDER_H
#define STARS_IMAGE_RECORDER_H

namespace media {

class image_recorder {
public:
    image_recorder() = default;
    ~image_recorder() = default;

private:
    image_recorder(image_recorder&&) = delete;
    image_recorder(const image_recorder&) = delete;
    image_recorder& operator=(image_recorder&&) = delete;
    image_recorder& operator=(const image_recorder&) = delete;
};

} //namespace media

#endif //STARS_IMAGE_RECORDER_H
