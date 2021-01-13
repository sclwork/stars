//
// Created by scliang on 1/13/21.
//

#include "log.h"
#include "camera.h"

#define log_d(...)  LOG_D("Media-Native:yuv2argb", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:yuv2argb", __VA_ARGS__)

namespace media {

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

// This value is 2 ^ 18 - 1, and is used to clamp the RGB values before their
// ranges
// are normalized to eight bits.
const int kMaxChannelValue = 262143;

static void yuv2argb_pixel(struct img_args &img_args) {
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

static void yuv2argb_out(struct img_args &img_args) {
    if (img_args.ori == 90) {
        img_args.cache[(img_args.img_width - 1 - img_args.y) +
                       img_args.x * img_args.img_width] = img_args.argb;
    } else if (img_args.ori == 180) {
        img_args.cache[(img_args.img_height - 1 - img_args.y) *
                       img_args.img_width + (img_args.img_width - 1 - img_args.x)] = img_args.argb;
    } else if (img_args.ori == 270) {
        img_args.cache[(img_args.img_height - 1 - img_args.x) *
                       img_args.img_width + img_args.img_width - 1 - img_args.y] = img_args.argb;
    } else {
        img_args.cache[img_args.y * img_args.img_width + img_args.x] = img_args.argb;
    }
}

void yuv2argb(struct img_args &img_args) {
    for (img_args.y = 0; img_args.y < img_args.src_h; img_args.y++) {
        img_args.pY = img_args.yPixel + img_args.yStride *
                (img_args.y + img_args.srcRect.top) + img_args.srcRect.left;

        img_args.uv_row_start = img_args.uvStride * ((img_args.y + img_args.srcRect.top) >> 1);
        img_args.pU = img_args.uPixel + img_args.uv_row_start + (img_args.srcRect.left >> 1);
        img_args.pV = img_args.vPixel + img_args.uv_row_start + (img_args.srcRect.left >> 1);

        for (img_args.x = 0; img_args.x < img_args.src_w; img_args.x++) {
            img_args.uv_offset = (img_args.x >> 1) * img_args.uvPixelStride;
            img_args.nY = img_args.pY[img_args.x];
            img_args.nU = img_args.pU[img_args.uv_offset];
            img_args.nV = img_args.pV[img_args.uv_offset];
            yuv2argb_pixel(img_args);
            yuv2argb_out(img_args);
        }
    }
}

} //namespace media
