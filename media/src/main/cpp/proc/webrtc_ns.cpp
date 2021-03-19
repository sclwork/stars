//
// Created by Scliang on 3/19/21.
//

#include "proc.h"
#include "jni_log.h"
#include "webrtc_ns.h"

#define log_d(...)  LOG_D("Media-Native:webrtc_ns", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:webrtc_ns", __VA_ARGS__)

namespace media {
} //namespace media

media::webrtc_ns::webrtc_ns(int32_t m, uint32_t spr)
:mode(m), sample_rate(spr), ns(WebRtcNs_Create()), in(), out() {
    log_d("created. [%d,%d].", sample_rate, mode);
}

media::webrtc_ns::~webrtc_ns() {
    if (ns != nullptr) free(ns);
    ns = nullptr;
    log_d("release.");
}

void media::webrtc_ns::init() {
    if (WebRtcNs_Init(ns, sample_rate) != 0) {
        WebRtcNs_Free(ns);
        ns = nullptr;
        return;
    }

    if (WebRtcNs_set_policy(ns, mode) != 0) {
        WebRtcNs_Free(ns);
        ns = nullptr;
        return;
    }

    log_d("init success.");
}

void media::webrtc_ns::complete() {
    if (ns != nullptr) free(ns);
    ns = nullptr;
}

void media::webrtc_ns::encode_frame(std::shared_ptr<audio_frame> &&aud_frame) {
    if (aud_frame == nullptr) {
        return;
    }

    int length = 0, count = 0, c, i;
    std::shared_ptr<uint16_t> sht = aud_frame->get(&length);
    uint16_t *buffer = sht.get();

    for (i = 0; i < length; i += BUF_LEN) {
        if (i + BUF_LEN <= length) {
            c = BUF_LEN;
        } else {
            c = length - i;
        }

        memset(in, 0, sizeof(uint16_t) *BUF_LEN);
        memcpy(in, buffer + i, sizeof(uint16_t) * c);
        count += c;

        const int16_t* pIn = (int16_t *)(&in);
        int16_t *pOut = (int16_t *)(&out);
        const int16_t* const* speechFrame = &pIn;
        int16_t* const* outFrame = &pOut;

        WebRtcNs_Analyze(ns, pIn);
        WebRtcNs_Process(ns, speechFrame, 1, outFrame);

        memcpy(buffer + i, out, sizeof(int16_t) * c);
        memset(out, 0, sizeof(uint16_t) *BUF_LEN);
    }

    aud_frame->set(sht, length);
//    log_d("encode_frame frm_size: %d, ns_size: %d.", length, count);
}
