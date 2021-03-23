//
// Created by Scliang on 3/19/21.
//

#ifndef STARS_WEBRTC_NS_H
#define STARS_WEBRTC_NS_H

#include <mutex>
#include <string>
#include "ns.h"
#include "proc_frame.h"
#include "audio_frame.h"
#include "loop/concurrent_queue.h"

namespace media {

const int32_t BUF_LEN = 160; // uint16_t
const int32_t FRM_LEN = 2048; // uint16_t

class webrtc_ns {
public:
    webrtc_ns(int32_t m, uint32_t spr);
    ~webrtc_ns();

public:
    /**
     * init/start ns
     */
    void init();
    /**
     * complete/stop ns
     */
    void complete();
    /**
     * ns audio frame
     * @param aud_frame audio frame
     */
    void encode_frame(std::shared_ptr<audio_frame> &aud_frame);
    /**
     * get encode completed frame
     * @return audio frame
     */
    std::shared_ptr<audio_frame> get_encoded_frame();

private:
    webrtc_ns(webrtc_ns&&) = delete;
    webrtc_ns(const webrtc_ns&) = delete;
    webrtc_ns& operator=(webrtc_ns&&) = delete;
    webrtc_ns& operator=(const webrtc_ns&) = delete;

private:
    int32_t   mode;
    int32_t   in_offset;
    int32_t   frm_offset;
    uint32_t  sample_rate;
    NsHandle *ns;
    uint16_t  in[BUF_LEN];
    uint16_t  out[BUF_LEN];
    uint16_t *frm_cache;
    std::shared_ptr<moodycamel::ConcurrentQueue<frame>> frameQ;
};

} //namespace media

#endif //STARS_WEBRTC_NS_H
