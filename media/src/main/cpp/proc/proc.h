//
// Created by Scliang on 3/10/21.
//

#ifndef STARS_PROC_H
#define STARS_PROC_H

#include <string>

namespace media {

static inline bool startWith(const std::string &str, const std::string &head) {
    return str.compare(0, head.size(), head) == 0;
}

static inline bool endWith(const std::string &str, const std::string &tail) {
    return str.compare(str.size() - tail.size(), tail.size(), tail) == 0;
}

static inline void make_dsi(uint32_t sampling_frequency_index, uint32_t channel_configuration, uint8_t *dsi) {
    uint32_t object_type = 2; // AAC LC by default 
    dsi[0] = (object_type << 3) | (sampling_frequency_index >> 1);
    dsi[1] = ((sampling_frequency_index & 1) << 7) | (channel_configuration << 3);
}

static inline uint32_t get_sr_index(int32_t sample_rate) {
    switch (sample_rate) {
        //  0: 96000 Hz
        case 96000: return 0;
        //  1: 88200 Hz
        case 88200: return 1;
        //  2: 64000 Hz
        case 64000: return 2;
        //  3: 48000 Hz
        case 48000: return 3;
        //  4: 44100 Hz
        case 44100: return 4;
        //  5: 32000 Hz
        case 32000: return 5;
        //  6: 24000 Hz
        case 24000: return 6;
        //  7: 22050 Hz
        case 22050: return 7;
        //  8: 16000 Hz
        case 16000: return 8;
        //  9: 12000 Hz
        case 12000: return 9;
        // 10: 11025 Hz
        case 11025: return 10;
        // 11: 8000 Hz
        case 8000: return 11;
        // 12: 7350 Hz
        case 7350: return 12;
        // 13: Other
        default: return 13;
    }
}

} //namespace media

#endif //STARS_PROC_H
