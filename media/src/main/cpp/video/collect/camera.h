//
// Created by scliang on 1/8/21.
//

#ifndef STARS_CAMERA_H
#define STARS_CAMERA_H

#include <vector>
#include <string>
#include <media/NdkImageReader.h>
#include <camera/NdkCameraDevice.h>
#include "image_cache.h"

namespace media {

enum RecState {
    None,
    Previewing,
    Recording,
    Paused_Preview,
    Paused_Record,
};

// get_latest_image tmp args
struct img_args {
    int32_t x, y, format, planeCount, yStride, uvStride, yLen, uLen, vLen, uvPixelStride;
    int32_t src_w, src_h, img_width, img_height, uv_row_start, uv_offset, nY, nU, nV, nR, nG, nB, ori;
    uint8_t *yPixel, *uPixel, *vPixel, *pY, *pU, *pV;
    AImageCropRect srcRect;
    uint32_t *cache, argb;
    AImage *image;
};

void yuv2argb(struct img_args &img_args);

class camera {
public:
    camera(std::string id, int32_t fps);
    ~camera();

public:
    static void enumerate(std::vector<std::shared_ptr<camera>> &cams);

public:
    std::string get_id();
    std::shared_ptr<image_cache> get_latest_image();

public:
    bool preview(int32_t req_w, int32_t req_h);
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
    RecState state;
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
    ///////////////////////////////////
    std::shared_ptr<image_cache> img_cache;
};

} //namespace media

#endif //STARS_CAMERA_H
