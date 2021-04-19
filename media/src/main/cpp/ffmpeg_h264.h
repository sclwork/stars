//
// Created by Scliang on 3/23/21.
//

#ifndef STARS_FFMPEG_H264_H
#define STARS_FFMPEG_H264_H

#define HAVE_IMAGE_STREAM
#define HAVE_AUDIO_STREAM

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
#include "ffmpeg_args.h"
#include "image_frame.h"
#include "audio_frame.h"

namespace media {

class ffmpeg_h264 {
public:
    ffmpeg_h264(image_args &&img, audio_args &&aud);
    virtual ~ffmpeg_h264();

public:
    virtual void init() = 0;
    virtual void complete();
    virtual void encode_image_frame(const image_frame& img_frame);
    virtual void encode_audio_frame(const audio_frame& aud_frame);

protected:
    void on_init_all(const std::string &format, const std::string &out_name);
    void on_free_all();

protected:
    image_args  image;
    audio_args  audio;
    //////////////////////////
    int64_t     i_pts;
    int64_t     a_pts;
    int32_t     a_encode_offset;
    int32_t     a_encode_length;
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

class ffmpeg_mp4 : public ffmpeg_h264 {
public:
    ffmpeg_mp4(std::string &&n, image_args &&img, audio_args &&aud);
    ~ffmpeg_mp4();

public:
    void init() override;

private:
    ffmpeg_mp4(ffmpeg_mp4&&) = delete;
    ffmpeg_mp4(const ffmpeg_mp4&) = delete;
    ffmpeg_mp4& operator=(ffmpeg_mp4&&) = delete;
    ffmpeg_mp4& operator=(const ffmpeg_mp4&) = delete;

private:
    std::string name;
};

class ffmpeg_rtmp : public ffmpeg_h264 {
public:
    ffmpeg_rtmp(std::string &&f, std::string &&n, image_args &&img, audio_args &&aud);
    ~ffmpeg_rtmp();

public:
    void init() override;

private:
    ffmpeg_rtmp(ffmpeg_rtmp&&) = delete;
    ffmpeg_rtmp(const ffmpeg_rtmp&) = delete;
    ffmpeg_rtmp& operator=(ffmpeg_rtmp&&) = delete;
    ffmpeg_rtmp& operator=(const ffmpeg_rtmp&) = delete;

private:
    std::string file;
    std::string name;
};

} //namespace media

#endif //STARS_FFMPEG_H264_H
