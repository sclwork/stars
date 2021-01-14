//
// Created by scliang on 1/11/21.
//

#ifndef STARS_IMAGE_FRAME_H
#define STARS_IMAGE_FRAME_H

#include <cstdint>

#define DRAW_TEST_FRAME 0

namespace media {

class image_frame {
public:
    image_frame(int32_t w, int32_t h);
    image_frame(const image_frame &frame);
    ~image_frame();

public:
    bool same_size(int32_t w, int32_t h) const;
    bool available() const;

public:
    void get(int32_t *out_w, int32_t *out_h, uint32_t **out_cache = nullptr) const;

#if DRAW_TEST_FRAME
public:
    void setup_test_data(int32_t w, int32_t h);
#endif

private:
    image_frame(image_frame&&) = delete;
    image_frame& operator=(image_frame&&) = delete;
    image_frame& operator=(const image_frame&) = delete;

private:
    int32_t width{};
    int32_t height{};
    uint32_t *cache;
};

} //namespace media

#endif //STARS_IMAGE_FRAME_H
