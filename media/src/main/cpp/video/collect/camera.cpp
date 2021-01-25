//
// Created by scliang on 1/8/21.
//

#include <memory>
#include <utility>
#include <vector>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraManager.h>
#include "log.h"
#include "camera.h"

#define log_d(...)  LOG_D("Media-Native:camera", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:camera", __VA_ARGS__)

namespace media {

extern void yuv2argb(struct img_args &img_args);

} //namespace media

void media::camera::enumerate(std::vector<std::shared_ptr<camera>> &cams) {
    ACameraManager *manager = ACameraManager_create();
    if (manager == nullptr) {
        return;
    }

    camera_status_t status;
    ACameraIdList *cameraIdList = nullptr;
    status = ACameraManager_getCameraIdList(manager, &cameraIdList);
    if (status != ACAMERA_OK) {
        log_e("Failed to get camera id list (reason: %d).", status);
        ACameraManager_delete(manager);
        return;
    }

    if (cameraIdList == nullptr || cameraIdList->numCameras < 1) {
        log_e("No camera device detected.");
        if (cameraIdList)
            ACameraManager_deleteCameraIdList(cameraIdList);
        ACameraManager_delete(manager);
        return;
    }

    cams.clear();
    for (int32_t i = 0; i < cameraIdList->numCameras; i++) {
        cams.push_back(std::make_shared<camera>(std::string(cameraIdList->cameraIds[i]), 25));
    }

    ACameraManager_delete(manager);
}

media::camera::camera(std::string id, int32_t fps)
:img_args(), id(std::move(id)), state(RecState::None), dev(nullptr),
fps_req(fps), fps_range(), ori(0), af_mode(ACAMERA_CONTROL_AF_MODE_OFF),
reader(nullptr), window(nullptr), cap_request(nullptr), out_container(nullptr),
out_session(nullptr), cap_session(nullptr), out_target(nullptr),
ds_callbacks({nullptr, onDisconnected, onError}),
css_callbacks({nullptr, onClosed, onReady, onActive}) {
    log_d("created. %s", this->id.c_str());
}

media::camera::~camera() {
    close();
    log_d("release. %s", id.c_str());
}

void media::camera::get_latest_image(std::shared_ptr<media::image_frame> &frame) {
    media_status_t status = AImageReader_acquireLatestImage(reader, &img_args.image);
    if (status != AMEDIA_OK) {
        return;
    }

    status = AImage_getFormat(img_args.image, &img_args.format);
    if (status != AMEDIA_OK || img_args.format != AIMAGE_FORMAT_YUV_420_888) {
        AImage_delete(img_args.image);
        return;
    }

    status = AImage_getNumberOfPlanes(img_args.image, &img_args.plane_count);
    if (status != AMEDIA_OK || img_args.plane_count != 3) {
        AImage_delete(img_args.image);
        return;
    }

    AImage_getPlaneRowStride(img_args.image, 0, &img_args.y_stride);
    AImage_getPlaneRowStride(img_args.image, 1, &img_args.vu_stride);
    AImage_getPlaneData(img_args.image, 0, &img_args.y_pixel, &img_args.y_len);
    AImage_getPlaneData(img_args.image, 1, &img_args.v_pixel, &img_args.v_len);
    AImage_getPlaneData(img_args.image, 2, &img_args.u_pixel, &img_args.u_len);
    AImage_getPlanePixelStride(img_args.image, 1, &img_args.vu_pixel_stride);

    AImage_getCropRect(img_args.image, &img_args.src_rect);
    img_args.src_w = img_args.src_rect.right - img_args.src_rect.left;
    img_args.src_h = img_args.src_rect.bottom - img_args.src_rect.top;
//    log_d("latest image size: %d,%d.", img_args.src_w, img_args.src_h);

    img_args.argb_pixel = (uint8_t *)malloc(sizeof(uint8_t) * img_args.src_w * img_args.src_h * 4);
    if (img_args.argb_pixel == nullptr) {
        if (img_args.vu_pixel == nullptr) {
            AImage_delete(img_args.image);
            return;
        }
    }

    img_args.vu_pixel = (uint8_t *)malloc(sizeof(uint8_t) * (img_args.v_len + img_args.u_len));
    if (img_args.vu_pixel == nullptr) {
        AImage_delete(img_args.image);
        free(img_args.argb_pixel);
        return;
    }

    memcpy(img_args.vu_pixel,                       img_args.v_pixel, sizeof(uint8_t) * img_args.v_len);
    memcpy(img_args.vu_pixel + img_args.v_len, img_args.u_pixel, sizeof(uint8_t) * img_args.u_len);

    if (img_args.ori == 90 || img_args.ori == 270) {
        img_args.img_width = img_args.src_h;
        img_args.img_height = img_args.src_w;
    } else {
        img_args.img_width = img_args.src_w;
        img_args.img_height = img_args.src_h;
    }

    img_args.frame = frame;
    img_args.frame->get(&img_args.frame_w, &img_args.frame_h, &img_args.frame_cache);
    img_args.wof = (img_args.frame_w - img_args.img_width) / 2;
    img_args.hof = (img_args.frame_h - img_args.img_height) / 2;
    yuv2argb(img_args);
    AImage_delete(img_args.image);
    free(img_args.vu_pixel);
    free(img_args.argb_pixel);
    img_args.image = nullptr;
}

bool media::camera::preview(int32_t req_w, int32_t req_h) {
    if (dev) {
        log_e("camera device is running.");
        return false;
    }

    if (id.empty()) {
        return false;
    }

    ACameraManager *mgr = ACameraManager_create();
    if (mgr == nullptr) {
        return false;
    }

    camera_status_t s;
    media_status_t ms;
    ACameraMetadata *metadata = nullptr;
    s = ACameraManager_getCameraCharacteristics(mgr, id.c_str(), &metadata);
    if (s != ACAMERA_OK) {
        log_e("Failed to get camera meta data of ID:%s.", id.c_str());
        ACameraManager_delete(mgr);
        return false;
    }

    get_fps(metadata);
    log_d("preview fps: %d,%d.", fps_range[0], fps_range[1]);
    get_ori(metadata);
    log_d("preview sensor orientation: %d.", ori);
    get_af_mode(metadata);
    log_d("select af mode: %d.", af_mode);
    int32_t width = 0, height = 0;
    get_size(metadata, req_w, req_h, &width, &height);
    log_d("preview size: %d,%d.", width, height);

    if (width <= 0 || height <= 0) {
        ACameraMetadata_free(metadata);
        ACameraManager_delete(mgr);
        return false;
    }

    if (reader) {
        AImageReader_setImageListener(reader, nullptr);
        AImageReader_delete(reader);
        ACameraMetadata_free(metadata);
        ACameraManager_delete(mgr);
        return false;
    }

    ms = AImageReader_new(width, height,
            AIMAGE_FORMAT_YUV_420_888, 2, &reader);
    if (ms != AMEDIA_OK) {
        log_e("Failed to new image reader.");
        ACameraMetadata_free(metadata);
        ACameraManager_delete(mgr);
        return false;
    }

    if (window) {
        ACameraMetadata_free(metadata);
        ACameraManager_delete(mgr);
        return false;
    }

    ms = AImageReader_getWindow(reader, &window);
    if (ms != AMEDIA_OK) {
        log_e("Failed to get native window.");
        ACameraMetadata_free(metadata);
        ACameraManager_delete(mgr);
        return false;
    }

    ANativeWindow_acquire(window);
    s = ACameraManager_openCamera(mgr, id.c_str(), &ds_callbacks, &dev);
    if (s != ACAMERA_OK) {
        log_e("Failed[%d] to open camera device (id: %s).", s, id.c_str());
        ACameraMetadata_free(metadata);
        ACameraManager_delete(mgr);
        return false;
    }

    s = ACameraDevice_createCaptureRequest(dev, TEMPLATE_RECORD, &cap_request);
    if (s != ACAMERA_OK) {
        log_e("Failed to create capture request.");
        ACameraMetadata_free(metadata);
        ACameraManager_delete(mgr);
        return false;
    }

    log_d("Success to create capture request.");
    s = ACaptureSessionOutputContainer_create(&out_container);
    if (s != ACAMERA_OK) {
        log_e("Failed to create session output container.");
        ACameraMetadata_free(metadata);
        ACameraManager_delete(mgr);
        return false;
    }

    // delete / release
    ACameraMetadata_free(metadata);
    ACameraManager_delete(mgr);
    log_d("Success to open camera device [id: %s].", id.c_str());

    s = ACameraOutputTarget_create(window, &out_target);
    if (s != ACAMERA_OK) {
        log_e("Failed to create CameraOutputTarget.");
        return false;
    }

    s = ACaptureRequest_addTarget(cap_request, out_target);
    if (s != ACAMERA_OK) {
        log_e("Failed to add CameraOutputTarget.");
        return false;
    }

    s = ACaptureSessionOutput_create(window, &out_session);
    if (s != ACAMERA_OK) {
        log_e("Failed to create CaptureSessionOutput.");
        return false;
    }

    s = ACaptureSessionOutputContainer_add(out_container, out_session);
    if (s != ACAMERA_OK) {
        log_e("Failed to add CaptureSessionOutput.");
        return false;
    }

    s = ACameraDevice_createCaptureSession(dev, out_container, &css_callbacks, &cap_session);
    if (s != ACAMERA_OK) {
        log_e("Failed[%d] to create CaptureSession.", s);
        return false;
    }

    s = ACaptureRequest_setEntry_u8(cap_request, ACAMERA_CONTROL_AF_MODE, 1, &af_mode);
    if (s != ACAMERA_OK) {
        log_e("Failed to set af mode.");
    }
    s = ACaptureRequest_setEntry_i32(cap_request, ACAMERA_CONTROL_AE_TARGET_FPS_RANGE, 2, fps_range);
    if (s != ACAMERA_OK) {
        log_e("Failed to set fps.");
    }
    if (ori == 270) {
        uint8_t scene = ACAMERA_CONTROL_SCENE_MODE_FACE_PRIORITY;
        s = ACaptureRequest_setEntry_u8(cap_request, ACAMERA_CONTROL_SCENE_MODE, 1, &scene);
        if (s != ACAMERA_OK) {
            log_e("Failed to set scene mode.");
        }
    } else {
        uint8_t scene = ACAMERA_CONTROL_SCENE_MODE_DISABLED;
        s = ACaptureRequest_setEntry_u8(cap_request, ACAMERA_CONTROL_SCENE_MODE, 1, &scene);
        if (s != ACAMERA_OK) {
            log_e("Failed to set scene mode.");
        }
    }

    s = ACameraCaptureSession_setRepeatingRequest(cap_session, nullptr, 1, &cap_request, nullptr);
    if (s != ACAMERA_OK) {
        log_e("Failed to set RepeatingRequest.");
        return false;
    }

    state = RecState::Previewing;
    log_d("Success to start preview [id: %s]; req size: %d,%d; pre size: %d,%d.",
            id.c_str(), req_w, req_h, width, height);
    return true;
}

void media::camera::close() {
    if (cap_request) {
        ACaptureRequest_free(cap_request);
        cap_request = nullptr;
    }

    if (dev) {
        ACameraDevice_close(dev);
        dev = nullptr;
    }

    if (out_session) {
        ACaptureSessionOutput_free(out_session);
        out_session = nullptr;
    }

    if (out_container) {
        ACaptureSessionOutputContainer_free(out_container);
        out_container = nullptr;
    }

    if (reader) {
        AImageReader_setImageListener(reader, nullptr);
        AImageReader_delete(reader);
        reader = nullptr;
    }

    if (window) {
        ANativeWindow_release(window);
        window = nullptr;
    }

    state = RecState::None;
    log_d("Success to close CameraDevice id: %s.", id.c_str());
}

void media::camera::get_fps(ACameraMetadata *metadata) {
    if (metadata == nullptr) {
        return;
    }

    camera_status_t status;
    ACameraMetadata_const_entry entry;
    status = ACameraMetadata_getConstEntry(metadata,
            ACAMERA_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES, &entry);
    if (status != ACAMERA_OK) {
        return;
    }

    bool found = false;
    int32_t current_best_match = -1;
    for (int32_t i = 0; i < entry.count; i++) {
        int32_t min = entry.data.i32[i * 2 + 0];
        int32_t max = entry.data.i32[i * 2 + 1];
        if (fps_req == max) {
            if (min == max) {
                fps_range[0] = min;
                fps_range[1] = max;
                found = true;
            } else if (current_best_match >= 0) {
                int32_t current_best_match_min = entry.data.i32[current_best_match * 2 + 0];
                if (min > current_best_match_min) {
                    current_best_match = i;
                }
            } else {
                current_best_match = i;
            }
        }
    }

    if (!found) {
        if (current_best_match >= 0) {
            fps_range[0] = entry.data.i32[current_best_match * 2 + 0];
            fps_range[1] = entry.data.i32[current_best_match * 2 + 1];
        } else {
            fps_range[0] = entry.data.i32[0];
            fps_range[1] = entry.data.i32[1];
        }
    }
}

void media::camera::get_ori(ACameraMetadata *metadata) {
    if (metadata == nullptr) {
        return;
    }

    camera_status_t status;
    ACameraMetadata_const_entry entry;
    status = ACameraMetadata_getConstEntry(metadata, ACAMERA_SENSOR_ORIENTATION, &entry);
    if (status != ACAMERA_OK) {
        return;
    }

    ori = entry.data.i32[0];
    img_args.ori = ori;
}

void media::camera::get_af_mode(ACameraMetadata *metadata) {
    if (metadata == nullptr) {
        return;
    }

    camera_status_t status;
    ACameraMetadata_const_entry entry;
    status = ACameraMetadata_getConstEntry(metadata,
            ACAMERA_CONTROL_AF_AVAILABLE_MODES, &entry);
    if (status != ACAMERA_OK) {
        return;
    }

    if (entry.count <= 0) {
        af_mode = ACAMERA_CONTROL_AF_MODE_OFF;
    } else if (entry.count == 1) {
        af_mode = entry.data.u8[0];
    } else {
        uint8_t af_a = 0, af_b = 0;
        for (int32_t i = 0; i < entry.count; i++) {
            if (entry.data.u8[i] == ACAMERA_CONTROL_AF_MODE_CONTINUOUS_VIDEO) {
                af_a = ACAMERA_CONTROL_AF_MODE_CONTINUOUS_VIDEO;
            } else if (entry.data.u8[i] == ACAMERA_CONTROL_AF_MODE_AUTO) {
                af_b = ACAMERA_CONTROL_AF_MODE_AUTO;
            }
        }
        if (af_a != 0) {
            af_mode = af_a;
        } else if (af_b != 0) {
            af_mode = af_b;
        } else {
            af_mode = ACAMERA_CONTROL_AF_MODE_OFF;
        }
    }
}

void media::camera::get_size(ACameraMetadata *metadata,
        int32_t req_w, int32_t req_h, int32_t *out_w, int32_t *out_h) {
    if (metadata == nullptr) {
        return;
    }

    camera_status_t status;
    ACameraMetadata_const_entry entry;
    status = ACameraMetadata_getConstEntry(metadata,
            ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry);
    if (status != ACAMERA_OK) {
        return;
    }

    int32_t w, h, sub, min = 6000;
#if LOG_ABLE
    std::string s;
#endif
    for (int32_t i = 0; i < entry.count; i += 4) {
        int32_t input = entry.data.i32[i + 3];
        int32_t format = entry.data.i32[i + 0];
        if (input) {
            continue;
        }

        if (format == AIMAGE_FORMAT_YUV_420_888 || format == AIMAGE_FORMAT_JPEG) {
            w = entry.data.i32[i * 4 + 1];
            h = entry.data.i32[i * 4 + 2];
            if (w == 0 || h == 0 || w > 6000 || h > 6000 || w < 200 || h < 200 ||
                    w*h > 6000000 || w*h < 1000000) {
                continue;
            }
#if LOG_ABLE
            s.append("[").append(std::to_string(w)).append(",")
             .append(std::to_string(h)).append("],");
#endif
            sub = w - req_h;
            if (sub >= 0 && sub < min) {
                min = sub;
                *out_w = w;
                *out_h = h;
            }
        }
    }
#if LOG_ABLE
    log_d("filter preview size: %s.", s.c_str());
#endif

    if (*out_w == 0 || *out_h == 0) {
        *out_w = req_h;
        *out_h = req_w;
    }
}
