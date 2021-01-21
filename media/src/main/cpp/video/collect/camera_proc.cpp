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

#define log_d(...)  LOG_D("Media-Native:camera_proc", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:camera_proc", __VA_ARGS__)

/**
 * Helper function for YUV_420 to RGB conversion. Courtesy of Tensorflow
 * ImageClassifier Sample:
 * https://github.com/tensorflow/tensorflow/blob/master/tensorflow/examples/android/jni/yuv2rgb.cc
 * The difference is that here we have to swap UV plane when calling it.
 */
#ifndef MAX
#define MAX(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a > _b ? _a : _b;      \
  })
#define MIN(a, b)           \
  ({                        \
    __typeof__(a) _a = (a); \
    __typeof__(b) _b = (b); \
    _a < _b ? _a : _b;      \
  })
#endif

namespace media {

// This value is 2 ^ 18 - 1, and is used to clamp the RGB values before their
// ranges
// are normalized to eight bits.
const int kMaxChannelValue = 262143;

void yuv2argb_pixel(struct img_args &img_args) {
    img_args.nY -= 16;
    img_args.nU -= 128;
    img_args.nV -= 128;
    if (img_args.nY < 0) img_args.nY = 0;

    img_args.nR = (int)(1192 * img_args.nY + 1634 * img_args.nV);
    img_args.nG = (int)(1192 * img_args.nY - 833  * img_args.nV - 400 * img_args.nU);
    img_args.nB = (int)(1192 * img_args.nY + 2066 * img_args.nU);

    img_args.nR = MIN(kMaxChannelValue, MAX(0, img_args.nR));
    img_args.nG = MIN(kMaxChannelValue, MAX(0, img_args.nG));
    img_args.nB = MIN(kMaxChannelValue, MAX(0, img_args.nB));

    img_args.nR = (img_args.nR >> 10) & 0xff;
    img_args.nG = (img_args.nG >> 10) & 0xff;
    img_args.nB = (img_args.nB >> 10) & 0xff;

    img_args.argb = 0xff000000 | (img_args.nR << 16) | (img_args.nG << 8) | img_args.nB;
}

void yuv2argb_out(struct img_args &img_args) {
    if (img_args.ori == 90) {
        img_args.frame_index = (img_args.frame_w - 1 - img_args.frame_y) + img_args.frame_x * img_args.frame_w;
    } else if (img_args.ori == 180) {
        img_args.frame_index = (img_args.frame_h - 1 - img_args.frame_y) * img_args.frame_w + (img_args.frame_w - 1 - img_args.frame_x);
    } else if (img_args.ori == 270) {
        img_args.frame_index = (img_args.frame_h - 1 - img_args.frame_x) * img_args.frame_w + img_args.frame_w - 1 - img_args.frame_y;
    } else {
        img_args.frame_index = img_args.frame_y * img_args.frame_w + img_args.frame_x;
    }

    img_args.frame_cache[img_args.frame_index] = img_args.argb;
}

void yuv2argb_get_yuv(struct img_args &img_args) {
    img_args.pY = img_args.y_pixel + img_args.y_stride * (img_args.y + img_args.src_rect.top) + img_args.src_rect.left;
    img_args.uv_row_start = img_args.uv_stride * ((img_args.y + img_args.src_rect.top) >> 1);
    img_args.pU = img_args.u_pixel + img_args.uv_row_start + (img_args.src_rect.left >> 1);
    img_args.pV = img_args.v_pixel + img_args.uv_row_start + (img_args.src_rect.left >> 1);
}

void yuv2argb_set_yuv(struct img_args &img_args) {
    img_args.uv_offset = (img_args.x >> 1) * img_args.uv_pixel_stride;
    img_args.nY = img_args.pY[img_args.x];
    img_args.nU = img_args.pU[img_args.uv_offset];
    img_args.nV = img_args.pV[img_args.uv_offset];
    yuv2argb_pixel(img_args);
    yuv2argb_out(img_args);
}

void yuv2argb_all_in(struct img_args &img_args) {
    if (img_args.ori == 90 || img_args.ori == 270) {
        img_args.frame_y = img_args.wof;
        for (img_args.y = 0; img_args.y < img_args.src_h; img_args.y++) {
            yuv2argb_get_yuv(img_args);
            img_args.frame_x = img_args.hof;
            for (img_args.x = 0; img_args.x < img_args.src_w; img_args.x++) {
                yuv2argb_set_yuv(img_args);
                img_args.frame_x++;
            }
            img_args.frame_y++;
        }
    } else {
        img_args.frame_y = img_args.hof;
        for (img_args.y = 0; img_args.y < img_args.src_h; img_args.y++) {
            yuv2argb_get_yuv(img_args);
            img_args.frame_x = img_args.wof;
            for (img_args.x = 0; img_args.x < img_args.src_w; img_args.x++) {
                yuv2argb_set_yuv(img_args);
                img_args.frame_x++;
            }
            img_args.frame_y++;
        }
    }
}

void yuv2argb_all_out(struct img_args &img_args) {
    if (img_args.ori == 90 || img_args.ori == 270) {
        img_args.frame_y = 0;
        for (img_args.y = 0; img_args.y < img_args.src_h; img_args.y++) {
            if (img_args.y < -img_args.wof || img_args.y >= img_args.frame_w - img_args.wof) {
                continue;
            }
            yuv2argb_get_yuv(img_args);
            img_args.frame_x = 0;
            for (img_args.x = 0; img_args.x < img_args.src_w; img_args.x++) {
                if (img_args.x < -img_args.hof || img_args.x >= img_args.frame_h - img_args.hof) {
                    continue;
                }
                yuv2argb_set_yuv(img_args);
                img_args.frame_x++;
            }
            img_args.frame_y++;
        }
    } else {
        img_args.frame_y = 0;
        for (img_args.y = 0; img_args.y < img_args.src_h; img_args.y++) {
            if (img_args.y < -img_args.hof || img_args.y >= img_args.frame_h - img_args.hof) {
                continue;
            }
            yuv2argb_get_yuv(img_args);
            img_args.frame_x = 0;
            for (img_args.x = 0; img_args.x < img_args.src_w; img_args.x++) {
                if (img_args.x < -img_args.wof ||
                    img_args.x >= img_args.frame_w - img_args.wof) {
                    continue;
                }
                yuv2argb_set_yuv(img_args);
                img_args.frame_x++;
            }
            img_args.frame_y++;
        }
    }
}

void yuv2argb_w_out_h_in(struct img_args &img_args) {
    if (img_args.ori == 90 || img_args.ori == 270) {
        img_args.frame_y = 0;
        for (img_args.y = 0; img_args.y < img_args.src_h; img_args.y++) {
            if (img_args.y < -img_args.wof || img_args.y >= img_args.frame_w - img_args.wof) {
                continue;
            }
            yuv2argb_get_yuv(img_args);
            img_args.frame_x = img_args.hof;
            for (img_args.x = 0; img_args.x < img_args.src_w; img_args.x++) {
                yuv2argb_set_yuv(img_args);
                img_args.frame_x++;
            }
            img_args.frame_y++;
        }
    } else {
        img_args.frame_y = img_args.hof;
        for (img_args.y = 0; img_args.y < img_args.src_h; img_args.y++) {
            yuv2argb_get_yuv(img_args);
            img_args.frame_x = 0;
            for (img_args.x = 0; img_args.x < img_args.src_w; img_args.x++) {
                if (img_args.x < -img_args.wof || img_args.x >= img_args.frame_w - img_args.wof) {
                    continue;
                }
                yuv2argb_set_yuv(img_args);
                img_args.frame_x++;
            }
            img_args.frame_y++;
        }
    }
}

void yuv2argb_w_in_h_out(struct img_args &img_args) {
    if (img_args.ori == 90 || img_args.ori == 270) {
        img_args.frame_y = img_args.wof;
        for (img_args.y = 0; img_args.y < img_args.src_h; img_args.y++) {
            yuv2argb_get_yuv(img_args);
            img_args.frame_x = 0;
            for (img_args.x = 0; img_args.x < img_args.src_w; img_args.x++) {
                if (img_args.x < -img_args.hof || img_args.x >= img_args.frame_h - img_args.hof) {
                    continue;
                }
                yuv2argb_set_yuv(img_args);
                img_args.frame_x++;
            }
            img_args.frame_y++;
        }
    } else {
        img_args.frame_y = 0;
        for (img_args.y = 0; img_args.y < img_args.src_h; img_args.y++) {
            if (img_args.y < -img_args.hof || img_args.y >= img_args.frame_h - img_args.hof) {
                continue;
            }
            yuv2argb_get_yuv(img_args);
            img_args.frame_x = img_args.wof;
            for (img_args.x = 0; img_args.x < img_args.src_w; img_args.x++) {
                yuv2argb_set_yuv(img_args);
                img_args.frame_x++;
            }
            img_args.frame_y++;
        }
    }
}

void yuv2argb(struct img_args &img_args) {
    if (img_args.wof >= 0 && img_args.hof >= 0) {
        yuv2argb_all_in(img_args);
    } else if (img_args.wof < 0 && img_args.hof >= 0) {
        yuv2argb_w_out_h_in(img_args);
    } else if (img_args.wof >= 0 && img_args.hof < 0) {
        yuv2argb_w_in_h_out(img_args);
    } else if (img_args.wof < 0 && img_args.hof < 0) {
        yuv2argb_all_out(img_args);
    }
}

} //namespace media
