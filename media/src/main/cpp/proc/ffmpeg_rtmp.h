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
#include "libavutil/time.h"
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
    ffmpeg_rtmp(int32_t id, std::string &&n, image_args &&img, audio_args &&aud);
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
     * encode image frame and audio frame [to rtmp url]
     * @param img_frame image frame
     * @param aud_frame audio frame
     */
    void encode_frame(std::shared_ptr<image_frame> &&img_frame,
                      std::shared_ptr<audio_frame> &&aud_frame);

private:
    void init_image_encode();
    void close_image_encode();
    void encode_image_frame(int32_t w, int32_t h, const uint32_t* const data);

private:
    ffmpeg_rtmp(ffmpeg_rtmp&&) = delete;
    ffmpeg_rtmp(const ffmpeg_rtmp&) = delete;
    ffmpeg_rtmp& operator=(ffmpeg_rtmp&&) = delete;
    ffmpeg_rtmp& operator=(const ffmpeg_rtmp&) = delete;

private:
    int32_t          _id;
    //////////////////////////
    int64_t          i_pts;
    int64_t          st_time;
    //////////////////////////
    std::string      name;
    image_args       image;
    audio_args       audio;
    //////////////////////////
    AVCodecContext  *ic_ctx;
    AVFormatContext *if_ctx;
    AVStream        *i_stm;
    SwsContext      *i_sws_ctx;
    AVFrame         *i_rgb_frm;
    AVFrame         *i_yuv_frm;
};

} //namespace media

#endif //STARS_FFMPEG_RTMP_H
