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
:mode(m), in_offset(0), frm_offset(0), sample_rate(spr), ns(WebRtcNs_Create()), in(), out(),
frm_cache((uint16_t*)malloc(sizeof(uint16_t)*FRM_LEN)), aQ() {
    log_d("created. [%d,%d].", sample_rate, mode);
}

media::webrtc_ns::~webrtc_ns() {
    if (ns != nullptr) free(ns);
    ns = nullptr;
    if (frm_cache != nullptr) free(frm_cache);
    frm_cache = nullptr;
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

void media::webrtc_ns::encode_frame(const audio_frame &aud_frame) {
    int32_t length = 0, buf_of = 0, cp_count;
    std::shared_ptr<uint16_t> sht = aud_frame.get_sht(&length);
    uint16_t *buffer = sht.get();

    if (frm_cache == nullptr) {
        return;
    }

//    log_d("encode_frame frame length: %d.", length);

    while (true) {
        cp_count = BUF_LEN - in_offset;
        if (buf_of + cp_count <= length) {
            memcpy(in + in_offset, buffer + buf_of, sizeof(uint16_t) * cp_count);
            buf_of += cp_count;
//            log_d("encode_frame buf_of: %d, cp_count: %d.", buf_of, cp_count);

            const int16_t* pIn = (int16_t *)(&in);
            int16_t *pOut = (int16_t *)(&out);
            const int16_t* const* speechFrame = &pIn;
            int16_t* const* outFrame = &pOut;

            WebRtcNs_Analyze(ns, pIn);
            WebRtcNs_Process(ns, speechFrame, 1, outFrame);

            if (frm_offset + cp_count < FRM_LEN) {
                memcpy(frm_cache + frm_offset, out, sizeof(int16_t) * cp_count);
                frm_offset += cp_count;
            } else {
                int32_t cc = FRM_LEN - frm_offset;
                memcpy(frm_cache + frm_offset, out, sizeof(int16_t) * cc);
                audio_frame de_frm(FRM_LEN * 2);
                de_frm.set(frm_cache, FRM_LEN);
                aQ.enqueue(std::forward<audio_frame>(de_frm));
                int32_t rc = cp_count - cc;
                memcpy(frm_cache, out + cc, sizeof(int16_t) * rc);
                frm_offset = rc;
            }
            in_offset = 0;
        } else {
            in_offset = length - buf_of;
            memcpy(in, buffer + buf_of, sizeof(uint16_t) * in_offset);
//            log_d("encode_frame remain size: %d.", in_offset);
            break;
        }
    }
}

bool media::webrtc_ns::get_encoded_frame(audio_frame &frame) {
    bool res = aQ.try_dequeue(frame);
    return res;
}
