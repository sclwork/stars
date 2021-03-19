//
// Created by Scliang on 3/8/21.
//

#ifndef STARS_FFMPEG_RTMP_H
#define STARS_FFMPEG_RTMP_H

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libpostproc/postprocess.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

#include <mutex>
#include <string>
#include "ffmpeg.h"
#include "image_frame.h"
#include "audio_frame.h"

namespace media {

class ffmpeg_rtmp {
public:
    ffmpeg_rtmp(std::string &&f, std::string &&n, image_args &&img, audio_args &&aud);
    ~ffmpeg_rtmp();

public:
    /**
     * init/start rtmp streaming
     */
    void init();
    /**
     * complete/stop rtmp streaming
     */
    void complete();
    /**
     * encode image frame [to rtmp url]
     * @param img_frame image frame
     */
    void encode_image_frame(std::shared_ptr<image_frame> &&img_frame);
    /**
     * encode audio frame [to rtmp url]
     * @param aud_frame audio frame
     */
    void encode_audio_frame(std::shared_ptr<audio_frame> &&aud_frame);

private:
    void on_free_all();

private:
    ffmpeg_rtmp(ffmpeg_rtmp&&) = delete;
    ffmpeg_rtmp(const ffmpeg_rtmp&) = delete;
    ffmpeg_rtmp& operator=(ffmpeg_rtmp&&) = delete;
    ffmpeg_rtmp& operator=(const ffmpeg_rtmp&) = delete;

private:
    int64_t          i_pts;
    int64_t          a_pts;
    int32_t          a_encode_offset;
    int32_t          a_encode_length;
    //////////////////////////
    std::string      file;
    std::string      name;
    image_args       image;
    audio_args       audio;
    //////////////////////////
    AVFormatContext *vf_ctx;
    AVCodecContext  *ic_ctx;
    AVStream        *i_stm;
    SwsContext      *i_sws_ctx;
    AVFrame         *i_rgb_frm;
    AVFrame         *i_yuv_frm;
    AVCodecContext  *ac_ctx;
    AVStream        *a_stm;
    SwrContext      *a_swr_ctx;
    AVFrame         *a_frm;
    uint8_t         *a_encode_cache;
    AVBitStreamFilterContext *i_h264bsfc;
    AVBitStreamFilterContext *a_aac_adtstoasc;
};

} //namespace media

#endif //STARS_FFMPEG_RTMP_H
