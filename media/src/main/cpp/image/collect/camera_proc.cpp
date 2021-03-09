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
#include <libyuv.h>
#include "jni_log.h"
#include "camera.h"

#define log_d(...)  LOG_D("Media-Native:camera_proc", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:camera_proc", __VA_ARGS__)

namespace media {

void yuv2argb(struct img_args &img_args) {
    int32_t res = libyuv::Android420ToARGB(img_args.y_pixel, img_args.y_stride,
                                           img_args.u_pixel, img_args.u_stride, img_args.v_pixel, img_args.v_stride, img_args.vu_pixel_stride,
                                           img_args.argb_pixel, img_args.src_w * 4, img_args.src_w, img_args.src_h);
//    log_d("libyuv::NV21ToARGB: %d.", res);
    if (res != 0) {
        return;
    }

    libyuv::RotationModeEnum r;
    if (img_args.ori == 90) {
        r = libyuv::RotationModeEnum ::kRotate90;
    } else if (img_args.ori == 180) {
        r = libyuv::RotationModeEnum ::kRotate180;
    } else if (img_args.ori == 270) {
        r = libyuv::RotationModeEnum ::kRotate270;
    } else {
        r = libyuv::RotationModeEnum ::kRotate0;
    }

    res = libyuv::ARGBRotate(img_args.argb_pixel, img_args.src_w * 4,
                             img_args.dst_argb_pixel, img_args.img_width * 4, img_args.src_w, img_args.src_h, r);
//    log_d("libyuv::ARGBRotate: %d.", res);
    if (res != 0) {
        return;
    }

//    log_d("wof,hof: %d,%d.", img_args.wof, img_args.hof);
    if (img_args.wof >= 0 && img_args.hof >= 0) {
        for (int32_t i = 0; i < img_args.img_height; i++) {
            memcpy(img_args.frame_cache + ((i + img_args.hof) * img_args.frame_w + img_args.wof),
                   img_args.dst_argb_pixel + (i * img_args.img_width) * 4,
                   sizeof(uint8_t) * img_args.img_width * 4);
        }
    } else if (img_args.wof < 0 && img_args.hof >= 0) {
        for (int32_t i = 0; i < img_args.img_height; i++) {
            memcpy(img_args.frame_cache + ((i + img_args.hof) * img_args.frame_w),
                   img_args.dst_argb_pixel + (i * img_args.img_width - img_args.wof) * 4,
                   sizeof(uint8_t) * img_args.frame_w * 4);
        }
    } else if (img_args.wof >= 0 && img_args.hof < 0) {
        for (int32_t i = 0; i < img_args.frame_h; i++) {
            memcpy(img_args.frame_cache + (i * img_args.frame_w + img_args.wof),
                   img_args.dst_argb_pixel + ((i - img_args.hof) * img_args.img_width) * 4,
                   sizeof(uint8_t) * img_args.img_width * 4);
        }
    } else if (img_args.wof < 0 && img_args.hof < 0) {
        for (int32_t i = 0; i < img_args.frame_h; i++) {
            memcpy(img_args.frame_cache + (i * img_args.frame_w),
                   img_args.dst_argb_pixel + ((i - img_args.hof) * img_args.img_width - img_args.wof) * 4,
                   sizeof(uint8_t) * img_args.frame_w * 4);
        }
    }
}

} //namespace media
