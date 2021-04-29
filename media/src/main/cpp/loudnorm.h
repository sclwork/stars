//
// Created by Scliang on 3/18/21.
//

#ifndef STARS_LOUDNORM_H
#define STARS_LOUDNORM_H

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
#include "ai_args.h"
#include "audio_frame.h"

namespace media {

class loudnorm {
public:
    loudnorm(std::string &&pam, audio_args &&aud);
    ~loudnorm();

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
    loudnorm(loudnorm&&) = delete;
    loudnorm(const loudnorm&) = delete;
    loudnorm& operator=(loudnorm&&) = delete;
    loudnorm& operator=(const loudnorm&) = delete;

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

#endif //STARS_LOUDNORM_H
