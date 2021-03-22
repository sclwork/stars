//
// Created by Scliang on 3/18/21.
//

#include "proc.h"
#include "jni_log.h"
#include "ffmpeg_loudnorm.h"

#define log_d(...)  LOG_D("Media-Native:ffmpeg_loudnorm", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:ffmpeg_loudnorm", __VA_ARGS__)

namespace media {
} //namespace media

media::ffmpeg_loudnorm::ffmpeg_loudnorm(std::string &&pam, audio_args &&aud)
:param(pam), audio(aud), graph(nullptr), abuffer_ctx(nullptr),
loudnorm_ctx(nullptr), aformat_ctx(nullptr), abuffersink_ctx(nullptr),
en_frame(nullptr), de_frame(nullptr) {
    log_d("created [%s].", param.c_str());
}

media::ffmpeg_loudnorm::~ffmpeg_loudnorm() {
    log_d("release.");
}

void media::ffmpeg_loudnorm::init() {
    av_register_all();
    avcodec_register_all();

    graph = avfilter_graph_alloc();
    if (graph == nullptr) {
        log_e("avfilter_graph_alloc graph fail.");
        on_free_all();
        return;
    }

    const AVFilter *abuffer = avfilter_get_by_name("abuffer");
    if (abuffer == nullptr) {
        log_e("avfilter_get_by_name abuffer fail.");
        on_free_all();
        return;
    }

    const AVFilter *loudnorm = avfilter_get_by_name("loudnorm");
    if (loudnorm == nullptr) {
        log_e("avfilter_get_by_name loudnorm fail.");
        on_free_all();
        return;
    }

    const AVFilter *aformat = avfilter_get_by_name("aformat");
    if (aformat == nullptr) {
        log_e("avfilter_get_by_name aformat fail.");
        on_free_all();
        return;
    }

    const AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
    if (abuffersink == nullptr) {
        log_e("avfilter_get_by_name abuffersink fail.");
        on_free_all();
        return;
    }

    abuffer_ctx = avfilter_graph_alloc_filter(graph, abuffer, "src_buffer");
    if (abuffer_ctx == nullptr) {
        log_e("avfilter_graph_alloc_filter abuffer_ctx fail.");
        on_free_all();
        return;
    }

    loudnorm_ctx = avfilter_graph_alloc_filter(graph, loudnorm, "loudnorm");
    if (loudnorm_ctx == nullptr) {
        log_e("avfilter_graph_alloc_filter loudnorm_ctx fail.");
        on_free_all();
        return;
    }

    aformat_ctx = avfilter_graph_alloc_filter(graph, aformat, "out_aformat");
    if (aformat_ctx == nullptr) {
        log_e("avfilter_graph_alloc_filter aformat_ctx fail.");
        on_free_all();
        return;
    }

    abuffersink_ctx = avfilter_graph_alloc_filter(graph, abuffersink, "sink");
    if (abuffersink_ctx == nullptr) {
        log_e("avfilter_graph_alloc_filter abuffersink_ctx fail.");
        on_free_all();
        return;
    }

    char ch_layout[64];
    av_get_channel_layout_string(ch_layout, sizeof(ch_layout), audio.channels, av_get_default_channel_layout(audio.channels));
    av_opt_set(abuffer_ctx, "channel_layout", ch_layout, AV_OPT_SEARCH_CHILDREN);
    av_opt_set_sample_fmt(abuffer_ctx, "sample_fmt", AV_SAMPLE_FMT_S16, AV_OPT_SEARCH_CHILDREN);
    av_opt_set_q(abuffer_ctx, "time_base", {1, (int32_t)audio.sample_rate }, AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int(abuffer_ctx, "sample_rate", audio.sample_rate, AV_OPT_SEARCH_CHILDREN);
    av_opt_set_int(abuffer_ctx, "channels", audio.channels, AV_OPT_SEARCH_CHILDREN);
    if (avfilter_init_str(abuffer_ctx, nullptr) < 0) {
        log_e("avfilter_init_str abuffer_ctx fail.");
        on_free_all();
        return;
    }

    if (avfilter_init_str(loudnorm_ctx, param.c_str()) < 0) {
        log_e("avfilter_init_str loudnorm_ctx fail.");
        on_free_all();
        return;
    }

    char out_str[64];
    snprintf(out_str, sizeof(out_str),
            "sample_fmts=%s:sample_rates=%d:channel_layouts=0x%" PRIx64,
            av_get_sample_fmt_name(AV_SAMPLE_FMT_S16), audio.sample_rate,
            av_get_default_channel_layout(audio.channels));
    if (avfilter_init_str(aformat_ctx, out_str) < 0) {
        log_e("avfilter_init_str aformat_ctx fail.");
        on_free_all();
        return;
    }

    if (avfilter_init_str(abuffersink_ctx, nullptr) < 0) {
        log_e("avfilter_init_str abuffersink_ctx fail.");
        on_free_all();
        return;
    }

    int32_t res =       avfilter_link(abuffer_ctx, 0, loudnorm_ctx, 0);
    if (res >= 0) res = avfilter_link(loudnorm_ctx, 0, aformat_ctx, 0);
    if (res >= 0) res = avfilter_link(aformat_ctx, 0, abuffersink_ctx, 0);
    if (res < 0) {
        log_e("avfilter_link fail.");
        on_free_all();
        return;
    }

    if ((res = avfilter_graph_config(graph, nullptr)) < 0) {
        char err[64];
        av_strerror(res, err, 64);
        log_e("avfilter_graph_config fail: [%d] %s", res, err);
        on_free_all();
        return;
    }

    en_frame = av_frame_alloc();
    if (en_frame == nullptr) {
        log_e("av_frame_alloc en_frame fail");
        on_free_all();
        return;
    }

    en_frame->nb_samples = 1024;
    en_frame->sample_rate = audio.sample_rate;
    en_frame->format = AV_SAMPLE_FMT_S16;
    en_frame->channel_layout = av_get_default_channel_layout(audio.channels);
    res = av_frame_get_buffer(en_frame, 0);
    if (res < 0) {
        log_e("av_frame_get_buffer en_frame fail: %d", res);
        on_free_all();
        return;
    }

    res = av_frame_make_writable(en_frame);
    if (res < 0) {
        log_e("av_frame_make_writable en_frame fail: %d", res);
        on_free_all();
        return;
    }

    de_frame = av_frame_alloc();
    if (de_frame == nullptr) {
        log_e("av_frame_alloc de_frame fail");
        on_free_all();
        return;
    }

    log_d("init success.");
}

void media::ffmpeg_loudnorm::complete() {
    int32_t res = av_buffersrc_add_frame_flags(abuffer_ctx, nullptr,
            AV_BUFFERSRC_FLAG_KEEP_REF|AV_BUFFERSRC_FLAG_PUSH);
    if (res < 0) {
        char err[64];
        av_strerror(res, err, 64);
        log_e("complete, av_buffersrc_add_frame_flags fail: (%d) %s", res, err);
    }
    on_free_all();
}

void media::ffmpeg_loudnorm::on_free_all() {
    if (de_frame != nullptr) av_frame_free(&de_frame);
    if (en_frame != nullptr) av_frame_free(&en_frame);
    if (abuffer_ctx != nullptr) avfilter_free(abuffer_ctx);
    if (loudnorm_ctx != nullptr) avfilter_free(loudnorm_ctx);
    if (aformat_ctx != nullptr) avfilter_free(aformat_ctx);
    if (abuffersink_ctx != nullptr) avfilter_free(abuffersink_ctx);
    if (graph != nullptr) avfilter_graph_free(&graph);
}

void media::ffmpeg_loudnorm::encode_frame(std::shared_ptr<audio_frame> &aud_frame) {
    int32_t count = 0; uint8_t *aud_data = nullptr;
    if (aud_frame != nullptr && aud_frame->available()) {
        aud_frame->get(&count, &aud_data);
    }

    if (count > 0 && aud_data != nullptr) {
        int32_t fSize = en_frame->linesize[0];
        int32_t c = count / fSize;
        if (count % fSize != 0) c++;
//        log_d("encode_frame en_frame fSize: %d, data size: %d, count: %d.", fSize, count, c);

        int32_t res;
        int32_t cp_count;
        int32_t offset = 0;
        int32_t frameCount = 0;
        for (int32_t i = 0; i < c; ++i) {
            if (i != 0 && i == c - 1) {
                cp_count = count % fSize;
            } else {
                cp_count = fSize;
            }

//            log_d("encode_frame en_frame cp_count: %d.", cp_count);
            memset(en_frame->data[0], 0, cp_count);
            memcpy(en_frame->data[0], aud_data + offset, sizeof(uint8_t) * cp_count);
            en_frame->pts = frameCount++;

            res = av_buffersrc_add_frame_flags(abuffer_ctx, en_frame,
                    AV_BUFFERSRC_FLAG_KEEP_REF|AV_BUFFERSRC_FLAG_PUSH);
            if (res < 0) {
                char err[64];
                av_strerror(res, err, 64);
                log_e("encode_frame av_buffersrc_add_frame_flags fail: (%d) %s", res, err);
                break;
            }

            offset += cp_count;
        }
    }
}

std::shared_ptr<media::audio_frame> media::ffmpeg_loudnorm::get_encoded_frame() {
    int32_t res = av_buffersink_get_frame(abuffersink_ctx, de_frame);
    if (res == AVERROR_EOF) {
        char err[64];
        av_strerror(res, err, 64);
//                log_e("encode_frame av_buffersink_get_frame fail: [%d] %s", res, err);
        return std::shared_ptr<audio_frame>(nullptr);
    }
    if (res < 0) {
        char err[64];
        av_strerror(res, err, 64);
//                log_e("encode_frame av_buffersink_get_frame fail: [%d] %s", res, err);
        return std::shared_ptr<audio_frame>(nullptr);
    }

    uint8_t *aud_data = nullptr;
    auto frm = std::make_shared<audio_frame>(de_frame->linesize[0]);
    frm->get(nullptr, &aud_data);
    memcpy(aud_data, de_frame->data[0], sizeof(uint8_t) * de_frame->linesize[0]);
    av_frame_unref(de_frame);

    return frm;
}
