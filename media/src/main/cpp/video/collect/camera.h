//
// Created by scliang on 1/8/21.
//

#ifndef STARS_CAMERA_H
#define STARS_CAMERA_H

#include <vector>
#include <string>
#include <media/NdkImageReader.h>
#include <camera/NdkCameraDevice.h>

namespace media {

enum RecState {
    None,
    Previewing,
    Recording,
    Paused_Preview,
    Paused_Record,
};

class camera {
public:
    camera(std::string id, int32_t fps);
    ~camera();

public:
    static void enumerate(std::vector<std::shared_ptr<camera>> &cams);

public:
    std::string get_id();
    int32_t get_latest_image();

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
    uint32_t *img_cache;
    int32_t img_width;
    int32_t img_height;
};

} //namespace media

#endif //STARS_CAMERA_H
