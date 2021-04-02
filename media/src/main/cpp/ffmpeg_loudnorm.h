//
// Created by Scliang on 3/18/21.
//

#ifndef STARS_FFMPEG_LOUDNORM_H
#define STARS_FFMPEG_LOUDNORM_H

#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libpostproc/postprocess.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

#include <mutex>
#include <string>
#include "ffmpeg_args.h"
#include "audio_frame.h"

namespace media {

class ffmpeg_loudnorm {
public:
    ffmpeg_loudnorm(std::string &&pam, audio_args &&aud);
    ~ffmpeg_loudnorm();

public:
    /**
     * init/start loudnorm
     */
    void init();
    /**
     * complete/stop loudnorm
     */
    void complete();
    /**
     * encode audio frame
     * @param frame audio frame
     */
    void encode_frame(const audio_frame &frame);
    /**
     * get encode completed frame
     * @return audio frame
     */
    bool get_encoded_frame(audio_frame &frame);

private:
    void on_free_all();

private:
    ffmpeg_loudnorm(ffmpeg_loudnorm&&) = delete;
    ffmpeg_loudnorm(const ffmpeg_loudnorm&) = delete;
    ffmpeg_loudnorm& operator=(ffmpeg_loudnorm&&) = delete;
    ffmpeg_loudnorm& operator=(const ffmpeg_loudnorm&) = delete;

private:
    std::string      param;
    audio_args       audio;
    //////////////////////////
    AVFilterGraph   *graph;
    AVFilterContext *abuffer_ctx;
    AVFilterContext *loudnorm_ctx;
    AVFilterContext *aformat_ctx;
    AVFilterContext *abuffersink_ctx;
    AVFrame         *en_frame;
    AVFrame         *de_frame;
};

} //namespace media

#endif //STARS_FFMPEG_LOUDNORM_H
