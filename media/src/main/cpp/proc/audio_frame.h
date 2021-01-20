//
// Created by scliang on 1/19/21.
//

#ifndef STARS_AUDIO_FRAME_H
#define STARS_AUDIO_FRAME_H

#include <cstdint>

namespace media {

class audio_recorder;

class audio_frame {
public:
    audio_frame(int32_t size);
    audio_frame(const audio_frame &frame);
    ~audio_frame();

public:
    bool available() const;
    void get(int32_t *out_size, int8_t **out_cache = nullptr) const;
    std::shared_ptr<int16_t> get(int32_t *out_size);

private:
    audio_frame(audio_frame&&) = delete;
    audio_frame& operator=(audio_frame&&) = delete;
    audio_frame& operator=(const audio_frame&) = delete;

private:
    friend class audio_recorder;

private:
    bool is_copy;
    /////////////////
    int32_t size;
    int8_t *cache;
    ///////////////////
    int32_t cp_offset;
};

} //namespace media

#endif //STARS_AUDIO_FRAME_H