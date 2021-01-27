//
// Created by scliang on 1/14/21.
//

#ifndef STARS_FFMPEG_H
#define STARS_FFMPEG_H

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
#include "image_frame.h"
#include "audio_frame.h"

namespace media {

typedef struct ff_image_args {
    uint32_t width, height, channels, fps, frame_size;
    void update_frame_size() { frame_size = width*height*channels; }
} ff_image_args;

typedef struct ff_audio_args {
    uint32_t channels, sample_rate, frame_size;
} ff_audio_args;

class ffmpeg_mp4 {
public:
    ffmpeg_mp4(std::string &&n, ff_image_args &&img, ff_audio_args &&aud);
    ~ffmpeg_mp4();

public:
    void reset_tmp_files();
    void append_av_frame(std::shared_ptr<image_frame> &&img_frame,
                         std::shared_ptr<audio_frame> &&aud_frame);

public:
    void init_image_encode();
    void init_audio_encode();
    void close_image_encode();
    void close_audio_encode();

private:
    void encode_image_frame(int32_t w, int32_t h, uint32_t *data);
    void encode_audio_frame();

private:
    ffmpeg_mp4(ffmpeg_mp4&&) = delete;
    ffmpeg_mp4(const ffmpeg_mp4&) = delete;
    ffmpeg_mp4& operator=(ffmpeg_mp4&&) = delete;
    ffmpeg_mp4& operator=(const ffmpeg_mp4&) = delete;

private:
    std::string _tmp;
    //////////////////////////
    std::string name;
    ff_image_args image;
    ff_audio_args audio;
    //////////////////////////
    std::string f_rgb_name;
    std::string f_264_name;
    std::string f_pcm_name;
    std::string f_aac_name;
    //////////////////////////
    int64_t          pts;
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
};

class ffmpeg {
public:
    ffmpeg();
    ~ffmpeg();

public:
    static void video_encode_idle(std::shared_ptr<ffmpeg>      &&ff);
    static void video_encode_frame(std::string                  &&n,
                                   ff_image_args                &&img,
                                   ff_audio_args                &&aud,
                                   std::shared_ptr<ffmpeg>      &&ff,
                                   std::shared_ptr<image_frame> &&img_frame,
                                   std::shared_ptr<audio_frame> &&aud_frame);

public:
    bool video_encode_available() const;

public:
    void video_encode_start(std::string   &&name,
                            ff_image_args &&image_args,
                            ff_audio_args &&audio_args);
    void video_encode_stop();
    void video_encode_frame(std::shared_ptr<image_frame> &&img_frame,
                            std::shared_ptr<audio_frame> &&aud_frame);

private:
    ffmpeg(ffmpeg&&) = delete;
    ffmpeg(const ffmpeg&) = delete;
    ffmpeg& operator=(ffmpeg&&) = delete;
    ffmpeg& operator=(const ffmpeg&) = delete;

private:
    std::shared_ptr<ffmpeg_mp4> mp4;
    mutable std::mutex          mp4_mutex;
};

} //namespace media

#endif //STARS_FFMPEG_H
