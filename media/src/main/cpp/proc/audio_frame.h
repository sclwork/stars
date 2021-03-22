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
    /**
     * check audio available
     */
    bool available() const;
    /**
     * get audio frame args/pcm data
     * @param out_size [out] audio pcm data size
     * @param out_cache [out] audio pcm data pointer
     */
    void get(int32_t *out_size, uint8_t **out_cache = nullptr) const;
    /**
     * get audio frame pcm short data
     * @param out_size [out] audio pcm short data size
     * @return audio frame pcm short data pointer
     */
    std::shared_ptr<uint16_t> get(int32_t *out_size);
    /**
     * set short data to audio frame pcm
     * @param sht audio pcm short data
     * @param length audio pcm short data size
     */
    void set(const uint16_t *sht, int32_t length);
    void set(const std::shared_ptr<uint16_t> &sht, int32_t length);

private:
    audio_frame(audio_frame&&) = delete;
    audio_frame& operator=(audio_frame&&) = delete;
    audio_frame& operator=(const audio_frame&) = delete;

private:
    friend class audio_recorder;

private:
    bool is_copy;
    /////////////////
    int32_t  size;
    uint8_t *cache;
    ///////////////////
    int32_t cp_offset;
};

} //namespace media

#endif //STARS_AUDIO_FRAME_H
