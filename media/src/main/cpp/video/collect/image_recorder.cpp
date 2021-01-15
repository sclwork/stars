//
// Created by scliang on 1/6/21.
//

#include "log.h"
#include "image_recorder.h"

#define log_d(...)  LOG_D("Media-Native:image_recorder", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:image_recorder", __VA_ARGS__)

namespace media {
} //namespace media

media::image_recorder::image_recorder()
:cache_args(), frame(nullptr), cams(), run_cam(nullptr) {
    log_d("created.");
    camera::enumerate(cams);
    log_d("--------------------");
    for (const auto& c : cams) {
        log_d("found cam %s", c->get_id().c_str());
    }
    log_d("--------------------");
}

media::image_recorder::~image_recorder() {
    run_cam = nullptr;
    log_d("release.");
}

int32_t media::image_recorder::camera_count() const {
    return cams.size();
}

void media::image_recorder::select_camera(int camera) {
    if (camera < 0 || camera >= cams.size()) {
        return;
    }

    if (run_cam) {
        run_cam->close();
        run_cam = nullptr;
    }

    if (frame == nullptr) {
        return;
    }

    int32_t w, h;
    frame->get(&w, &h);
    run_cam = cams[camera];
    run_cam->preview(w, h);
}

void media::image_recorder::update_size(int32_t w, int32_t h) {
    // check reset cache
    if (frame == nullptr || !frame->same_size(w, h)) {
        frame = std::make_shared<image_frame>(w, h);
    }
    // restart cam
    if (run_cam) {
        run_cam->close();
        run_cam->preview(w, h);
    }
}

std::shared_ptr<media::image_frame> media::image_recorder::collect_frame() {
    if (frame == nullptr || !frame->available() || run_cam == nullptr) {
        return frame;
    }

    frame->get(&cache_args.cache_width, &cache_args.cache_height, &cache_args.cache_cache);
    std::shared_ptr<media::image_frame> cam_cache = run_cam->get_latest_image();
    if (cam_cache == nullptr || !cam_cache->available()) {
        return frame;
    }

    cam_cache->get(&cache_args.img_width, &cache_args.img_height, &cache_args.img_cache);
    if (cache_args.img_cache == nullptr) {
        return frame;
    }

    cache_args.wof = (cache_args.cache_width - cache_args.img_width) / 2;
    cache_args.hof = (cache_args.cache_height - cache_args.img_height) / 2;
    if (cache_args.wof >= 0 && cache_args.hof >= 0) {
        for (int32_t i = 0; i < cache_args.img_height; i++) {
            memcpy(cache_args.cache_cache + ((i + cache_args.hof) * cache_args.cache_width + cache_args.wof),
                   cache_args.img_cache + (i * cache_args.img_width),
                   sizeof(uint32_t) * cache_args.img_width);
        }
    } else if (cache_args.wof < 0 && cache_args.hof >= 0) {
        for (int32_t i = 0; i < cache_args.img_height; i++) {
            memcpy(cache_args.cache_cache + ((i + cache_args.hof) * cache_args.cache_width),
                   cache_args.img_cache + (i * cache_args.img_width - cache_args.wof),
                   sizeof(uint32_t) * cache_args.cache_width);
        }
    } else if (cache_args.wof >= 0 && cache_args.hof < 0) {
        for (int32_t i = 0; i < cache_args.cache_height; i++) {
            memcpy(cache_args.cache_cache + (i * cache_args.cache_width + cache_args.wof),
                   cache_args.img_cache + ((i - cache_args.hof) * cache_args.img_width),
                   sizeof(uint32_t) * cache_args.img_width);
        }
    } else if (cache_args.wof < 0 && cache_args.hof < 0) {
        for (int32_t i = 0; i < cache_args.cache_height; i++) {
            memcpy(cache_args.cache_cache + (i * cache_args.cache_width),
                   cache_args.img_cache + ((i - cache_args.hof) * cache_args.img_width - cache_args.wof),
                   sizeof(uint32_t) * cache_args.cache_width);
        }
    }

    return frame;
}
