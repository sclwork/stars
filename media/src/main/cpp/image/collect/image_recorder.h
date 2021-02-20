//
// Created by scliang on 1/6/21.
//

#ifndef STARS_IMAGE_RECORDER_H
#define STARS_IMAGE_RECORDER_H

#include <mutex>
#include <string>
#include "proc/image_frame.h"
#include "camera.h"

namespace media {

class image_recorder {
public:
    image_recorder();
    ~image_recorder();

public:
    /**
     * @return camera count
     */
    int32_t camera_count() const;
    /**
     * @return true: selected camera and previewing
     */
    bool is_previewing() const;
    /**
     * select camera with index
     * @param camera index [0, camera_count)
     * @return true: select camera success
     */
    bool select_camera(int camera);
    /**
     * update image frame size
     * if image_frame is nullptr or not available or not same size then create new image_frame
     * @param w frame width
     * @param h frame height
     */
    void update_size(int32_t w, int32_t h);
    /**
     * from selected camera collect an image frame
     * @return collect success image_frame, all return image_frame is same address
     */
    std::shared_ptr<image_frame> collect_frame();

public:
    /**
     * @return selected camera fps
     */
    int32_t  get_fps() const;
    /**
     * @return image_frame width
     */
    uint32_t get_width() const;
    /**
     * @return image_frame height
     */
    uint32_t get_height() const;
    /**
     * @return image_frame image channels argb:4
     */
    uint32_t get_channels() const { return 4; }

private:
    image_recorder(image_recorder&&) = delete;
    image_recorder(const image_recorder&) = delete;
    image_recorder& operator=(image_recorder&&) = delete;
    image_recorder& operator=(const image_recorder&) = delete;

private:
    int32_t fps;
    bool previewing;
    std::shared_ptr<image_frame> frame;
    /////////////////////////////////////////
    std::vector<std::shared_ptr<camera>> cams;
    std::shared_ptr<camera>run_cam;
};

} //namespace media

#endif //STARS_IMAGE_RECORDER_H