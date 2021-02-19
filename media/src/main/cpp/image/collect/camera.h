//
// Created by scliang on 1/8/21.
//

#ifndef STARS_CAMERA_H
#define STARS_CAMERA_H

#include <vector>
#include <string>
#include <media/NdkImageReader.h>
#include <camera/NdkCameraDevice.h>
#include "proc/image_frame.h"

namespace media {

enum class RecState {
    None,
    Previewing,
};

// get_latest_image tmp args
struct img_args {
    int32_t wof, hof, frame_w, frame_h, i, j;
    int32_t x, y, format, plane_count, y_stride, u_stride, v_stride, vu_pixel_stride, y_len, u_len, v_len, ori;
    int32_t src_w, src_h, img_width, img_height;
    uint8_t *y_pixel, *u_pixel, *v_pixel, *argb_pixel, *dst_argb_pixel;
    std::shared_ptr<media::image_frame> frame;
    AImageCropRect src_rect;
    uint32_t *frame_cache, argb;
    AImage *image;
};

class camera {
public:
    camera(std::string id, int32_t fps);
    ~camera();

public:
    /**
     * check lc.id == rc.id
     * @param lc left camera
     * @param rc right camera
     * @return true: lc.id == rc.id
     */
    static bool equal(const camera &lc, const camera &rc);
    /**
     * enumerate all cameras
     * @param cams all cameras
     */
    static void enumerate(std::vector<std::shared_ptr<camera>> &cams);

public:
    /**
     * get latest image from camera
     * call after {@link preview}
     * @param frame [out] latest image frame
     */
    void get_latest_image(std::shared_ptr<media::image_frame> &frame);

public:
    /**
     * start camera preview
     * @param req_w requested image width
     * @param req_h requested image height
     * @param out_fps [out] camera fps
     * @return true: start preview success
     */
    bool preview(int32_t req_w, int32_t req_h, int32_t *out_fps);
    /**
     * close camera preview
     */
    void close();

private:
    camera(camera&&) = delete;
    camera(const camera&) = delete;
    camera& operator=(camera&&) = delete;
    camera& operator=(const camera&) = delete;

private:
    void get_fps(ACameraMetadata *metadata);
    void get_ori(ACameraMetadata *metadata);
    void get_af_mode(ACameraMetadata *metadata);

private:
    static void get_size(ACameraMetadata *metadata, int32_t req_w, int32_t req_h, int32_t *out_w, int32_t *out_h);
    static void onDisconnected(void *context, ACameraDevice *device) {}
    static void onError(void *context, ACameraDevice *device, int error) {}
    static void onActive(void *context, ACameraCaptureSession *session) {}
    static void onReady(void *context, ACameraCaptureSession *session) {}
    static void onClosed(void *context, ACameraCaptureSession *session) {}

private:
    struct img_args img_args;
    ///////////////////////////////////
    std::string id;
    std::atomic<RecState> state;
    ACameraDevice *dev;
    ///////////////////////////////////
    int32_t fps_req;
    int32_t fps_range[2];
    int32_t ori;
    uint8_t af_mode;
    ///////////////////////////////////
    AImageReader *reader;
    ANativeWindow *window;
    ACaptureRequest *cap_request;
    ACaptureSessionOutputContainer *out_container;
    ///////////////////////////////////
    ACaptureSessionOutput *out_session;
    ACameraCaptureSession *cap_session;
    ACameraOutputTarget *out_target;
    ///////////////////////////////////
    ACameraDevice_StateCallbacks ds_callbacks;
    ACameraCaptureSession_stateCallbacks css_callbacks;
};

} //namespace media

#endif //STARS_CAMERA_H
