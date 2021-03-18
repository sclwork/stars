//
// Created by scliang on 1/14/21.
//

#ifndef STARS_FFMPEG_MP4_H
#define STARS_FFMPEG_MP4_H

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

class ffmpeg_mp4 {
public:
    ffmpeg_mp4(std::string &&n, image_args &&img, audio_args &&aud);
    ~ffmpeg_mp4();

public:
    /**
     * init/start mp4 encoder
     */
    void init();
    /**
     * complete/stop mp4 encoder
     */
    void complete();
    /**
     * encode image frame and audio frame [to mp4 file]
     * @param img_frame image frame
     * @param aud_frame audio frame
     */
    void encode_frame(std::shared_ptr<image_frame> &&img_frame,
                      std::shared_ptr<audio_frame> &&aud_frame);

private:
    void on_free_all();
    void encode_ia_frame(int32_t w, int32_t h, const uint32_t* const img_data,
                         int32_t count, const uint8_t* const aud_data);

private:
    ffmpeg_mp4(ffmpeg_mp4&&) = delete;
    ffmpeg_mp4(const ffmpeg_mp4&) = delete;
    ffmpeg_mp4& operator=(ffmpeg_mp4&&) = delete;
    ffmpeg_mp4& operator=(const ffmpeg_mp4&) = delete;

private:
    std::string _tmp;
    //////////////////////////
    int64_t     i_pts;
    int64_t     a_pts;
    int32_t     a_encode_offset;
    int32_t     a_encode_length;
    //////////////////////////
    std::string name;
    image_args  image;
    audio_args  audio;
    //////////////////////////
    std::string f_rgb_name;
    std::string f_264_name;
    std::string f_pcm_name;
    std::string f_aac_name;
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

#endif //STARS_FFMPEG_MP4_H
