//
// Created by scliang on 1/11/21.
//

#ifndef STARS_IMAGE_FRAME_H
#define STARS_IMAGE_FRAME_H

#include <cstdint>

namespace media {

class image_frame;
class image_frame_ctx {
public:
    void *ctx;
    image_frame *frame;
};

typedef void (*IMAGE_FRAME_OP_CALLBACK)(image_frame_ctx *ctx);

class image_frame {
public:
    static image_frame *make_default(int32_t w, int32_t h);

public:
    image_frame(int32_t w, int32_t h);
    image_frame(const image_frame &frame);
    ~image_frame();

public:
    /**
     * check frame size
     * @param w frame width
     * @param h frame height
     * @return true: same size
     */
    bool same_size(int32_t w, int32_t h) const;
    /**
     * check image frame available
     */
    bool available() const;

public:
    /**
     * setup camera/image orientation
     * @param o orientation:[0|90|180|270]
     */
    void set_ori(int32_t o);
    /**
     * @return true: if camera/image orientation is 270
     */
    bool use_mirror() const;
    /**
     * get image frame args/data pointer
     * @param out_w [out] frame width
     * @param out_h [out] frame height
     * @param out_cache [out] frame data pointer
     */
    void get(int32_t *out_w, int32_t *out_h, uint32_t **out_cache = nullptr) const;

public:
    void set_op_callback(IMAGE_FRAME_OP_CALLBACK cb, void *ctx);
    void run_op_callback(image_frame *frame);

private:
    image_frame(image_frame&&) = delete;
    image_frame& operator=(image_frame&&) = delete;
    image_frame& operator=(const image_frame&) = delete;

private:
    bool is_copy;
    /////////////////
    int32_t ori;
    int32_t width;
    int32_t height;
    uint32_t *cache;
    ////////////////////////////////////
    IMAGE_FRAME_OP_CALLBACK op_callback;
    void *op_ctx;
};

} //namespace media

#endif //STARS_IMAGE_FRAME_H
