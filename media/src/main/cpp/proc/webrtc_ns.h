//
// Created by Scliang on 3/19/21.
//

#ifndef STARS_WEBRTC_NS_H
#define STARS_WEBRTC_NS_H

#include <mutex>
#include <string>
#include "ns.h"
#include "audio_frame.h"

namespace media {

const int32_t BUF_LEN = 160;

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
    void encode_frame(std::shared_ptr<audio_frame> &&aud_frame);

private:
    webrtc_ns(webrtc_ns&&) = delete;
    webrtc_ns(const webrtc_ns&) = delete;
    webrtc_ns& operator=(webrtc_ns&&) = delete;
    webrtc_ns& operator=(const webrtc_ns&) = delete;

private:
    int32_t   mode;
    uint32_t  sample_rate;
    NsHandle *ns;
    uint16_t  in[BUF_LEN];
    uint16_t  out[BUF_LEN];
};

} //namespace media

#endif //STARS_WEBRTC_NS_H
