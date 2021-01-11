//
// Created by scliang on 1/11/21.
//

#ifndef STARS_IMAGE_CACHE_H
#define STARS_IMAGE_CACHE_H

#include <cstdint>

#define DRAW_TEST_FRAME 1

namespace media {

class image_cache {
public:
    image_cache();
    ~image_cache();

public:
    bool update_size(int32_t w, int32_t h);
    bool same_size(int32_t w, int32_t h) const;
    bool available() const;

public:
    void get(int32_t *out_w, int32_t *out_h, uint32_t **out_cache) const;

#if DRAW_TEST_FRAME
public:
    void setup_test_data(int32_t w, int32_t h);
#endif

private:
    image_cache(image_cache&&) = delete;
    image_cache(const image_cache&) = delete;
    image_cache& operator=(image_cache&&) = delete;
    image_cache& operator=(const image_cache&) = delete;

private:
    int32_t width;
    int32_t height;
    uint32_t *cache;
};

} //namespace media

#endif //STARS_IMAGE_CACHE_H
