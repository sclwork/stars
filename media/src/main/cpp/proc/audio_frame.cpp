//
// Created by scliang on 1/19/21.
//

#include <string>
#include "jni_log.h"
#include "audio_frame.h"

#define log_d(...)  LOG_D("Media-Native:audio_frame", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:audio_frame", __VA_ARGS__)

namespace media {
} //namespace media

media::audio_frame::audio_frame(int32_t s)
:is_copy(false), size(s),
cache((uint8_t*)malloc(sizeof(uint8_t)*size)), cp_offset(0) {
//    log_d("created.");
    if (cache == nullptr) {
        log_e("malloc pcm cache fail.");
    }
}

media::audio_frame::audio_frame(const audio_frame &frame)
:is_copy(true), size(frame.size),
cache((uint8_t*)malloc(sizeof(uint8_t)*size)), cp_offset(0) {
//    log_d("created.");
    if (cache) {
        memcpy(cache, frame.cache, sizeof(int8_t)*size);
//        log_d("copy completed. %d.", size);
    }
}

media::audio_frame::~audio_frame() {
    if (cache) free(cache);
//    if (!is_copy) log_d("release.");
}

bool media::audio_frame::available() const {
    return cache != nullptr;
}

void media::audio_frame::get(int32_t *out_size, uint8_t **out_cache) const {
    if (out_size) *out_size = size;
    if (out_cache) *out_cache = cache;
}

std::shared_ptr<uint16_t> media::audio_frame::get(int32_t *out_size) {
    if (out_size) *out_size = size / 2;
    auto sa = new uint16_t[size / 2];
    std::shared_ptr<uint16_t> sht(sa,[](const uint16_t*p){delete[]p;});
    for (int32_t i = 0; i < size / 2; i++) {
        sa[i] = ((uint16_t)(cache[i * 2])     & 0xff) +
               (((uint16_t)(cache[i * 2 + 1]) & 0xff) << 8);
    }
    return sht;
}

void media::audio_frame::set(const std::shared_ptr<uint16_t>& sht, int32_t length) {
    if (sht != nullptr && length > 0 && length * 2 == size && cache != nullptr) {
        uint16_t *sd = sht.get();
        for (int32_t i = 0; i < length; i++) {
            cache[i * 2]     = (uint8_t) (sd[i]        & 0xff);
            cache[i * 2 + 1] = (uint8_t)((sd[i] >> 8)  & 0xff);
        }
    }
}
