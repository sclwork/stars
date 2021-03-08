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
    ffmpeg_mp4(int32_t id, std::string &&n, image_args &&img, audio_args &&aud);
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
    /*
     * reset tmp files:[.pcm, .acc, .argb, .yuv, .mp4]
     */
    void reset_tmp_files();
    /*
     * init ffmpeg image encode codec
     */
    void init_image_encode();
    /*
     * init ffmpeg audio encode codec
     */
    void init_audio_encode();
    /*
     * close ffmpeg image encode codec
     */
    void close_image_encode();
    /*
     * close ffmpeg audio encode codec
     */
    void close_audio_encode();

private:
    void encode_image_frame(int32_t w, int32_t h, const uint32_t* const data);
    void encode_audio_frame(int32_t count, const int8_t* const data);

private:
    ffmpeg_mp4(ffmpeg_mp4&&) = delete;
    ffmpeg_mp4(const ffmpeg_mp4&) = delete;
    ffmpeg_mp4& operator=(ffmpeg_mp4&&) = delete;
    ffmpeg_mp4& operator=(const ffmpeg_mp4&) = delete;

private:
    int32_t _id;
    std::string _tmp;
    //////////////////////////
    std::string name;
    image_args image;
    audio_args audio;
    //////////////////////////
    std::string f_rgb_name;
    std::string f_264_name;
    std::string f_pcm_name;
    std::string f_aac_name;
    //////////////////////////
    int64_t          i_pts;
    int64_t          a_pts;
    int32_t          a_encode_offset;
    int32_t          a_encode_length;
    //////////////////////////
    AVFormatContext *if_ctx;
    AVCodecContext  *ic_ctx;
    AVStream        *i_stm;
    AVFrame         *i_rgb_frm;
    AVFrame         *i_yuv_frm;
    SwsContext      *i_sws_ctx;
    AVFormatContext *af_ctx;
    AVCodecContext  *ac_ctx;
    AVStream        *a_stm;
    AVFrame         *a_frm;
    SwrContext      *a_swr_ctx;
    int8_t          *a_encode_cache;
};

} //namespace media

#endif //STARS_FFMPEG_MP4_H
