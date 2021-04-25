//
// Created by scliang on 1/11/21.
//

#ifndef STARS_IMAGE_FRAME_H
#define STARS_IMAGE_FRAME_H

#include <cstdint>
#include <opencv2/core/types.hpp>

namespace media {

class image_frame {
public:
    image_frame();
    image_frame(int32_t w, int32_t h);
    image_frame(image_frame&&) noexcept;
    image_frame(const image_frame &frame);
    image_frame& operator=(image_frame&&) noexcept;
    image_frame& operator=(const image_frame&) noexcept;
    ~image_frame();

public:
    /**
     * check frame size
     * @param w frame width
     * @param h frame height
     * @return true: same size
     */
    bool same_size(int32_t w, int32_t h) const
        { return w == width && h == height; }
    /**
     * check image frame available
     */
    bool available() const
        { return cache != nullptr; }

public:
    /**
     * update frame size
     * @param w frame width
     * @param h frame height
     * if w/h changed, remalloc data cache.
     */
    void update_size(int32_t w, int32_t h);
    /**
     * update faces in frame
     * @param fs
     */
    void update_faces(const std::vector<cv::Rect> &fs);
    void get_faces(std::vector<cv::Rect> &fs) const;
    /**
     * setup camera/image orientation
     * @param o orientation:[0|90|180|270]
     */
    void set_ori(int32_t o) { ori = o; }
    int32_t get_ori() const { return ori; }
    /**
     * @return true: if camera/image orientation is 270
     */
    bool use_mirror() const
        { return ori == 270; }
    /**
     * get image frame args/data pointer
     * @param out_w [out] frame width
     * @param out_h [out] frame height
     * @param out_cache [out] frame data pointer
     */
    void get(int32_t *out_w, int32_t *out_h, uint32_t **out_cache = nullptr) const;

private:
    bool is_copy;
    /////////////////
    int32_t ori;
    int32_t width;
    int32_t height;
    uint32_t *cache;
    /////////////////
    std::vector<cv::Rect> faces;
};

} //namespace media

#endif //STARS_IMAGE_FRAME_H
