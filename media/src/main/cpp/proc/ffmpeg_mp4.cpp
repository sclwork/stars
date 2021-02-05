//
// Created by scliang on 1/14/21.
//

#include <cstdio>
#include "log.h"
#include "loop/loop.h"
#include "ffmpeg.h"

#define log_d(...)  LOG_D("Media-Native:ffmpeg_mp4", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:ffmpeg_mp4", __VA_ARGS__)

namespace media {
} //namespace media

media::ffmpeg_mp4::ffmpeg_mp4(int32_t id, std::string &&n, ff_image_args &&img, ff_audio_args &&aud)
:_id(id), req_stop(false), _tmp(std::string(n).replace(n.find(".mp4"), 4, "")), name(n), image(img), audio(aud),
f_rgb_name(_tmp + "_" + std::to_string(id) + ".rgb"), f_264_name(_tmp + "_" + std::to_string(id) + ".h264"),
f_pcm_name(_tmp + "_" + std::to_string(id) + ".pcm"), f_aac_name(_tmp + "_" + std::to_string(id) + ".aac"), pts(0),
if_ctx(nullptr), ic_ctx(nullptr), i_stm(nullptr), i_rgb_frm(nullptr), i_yuv_frm(nullptr), i_sws_ctx(nullptr),
af_ctx(nullptr), ac_ctx(nullptr), a_stm(nullptr), a_frm(nullptr) {
    image.update_frame_size();
    log_d("[%d] created. [v:%d,%d,%d,%d],[a:%d,%d,%d],\n%s\n%s\n%s\n%s\n%s.",
          _id, image.width, image.height, image.channels, image.frame_size,
          audio.channels, audio.sample_rate, audio.frame_size,
          f_rgb_name.c_str(), f_264_name.c_str(), f_pcm_name.c_str(), f_aac_name.c_str(), name.c_str());
}

media::ffmpeg_mp4::~ffmpeg_mp4() {
    if (i_sws_ctx) sws_freeContext(i_sws_ctx);
    if (i_rgb_frm) av_frame_free(&i_rgb_frm);
    if (i_yuv_frm) av_frame_free(&i_yuv_frm);
    if (ic_ctx) avcodec_close(ic_ctx);
    if (ic_ctx) avcodec_free_context(&ic_ctx);
    if (if_ctx) avformat_free_context(if_ctx);
    if (a_frm) av_frame_free(&a_frm);
    if (ac_ctx) avcodec_close(ac_ctx);
    if (ac_ctx) avcodec_free_context(&ac_ctx);
    if (af_ctx) avformat_free_context(af_ctx);
    log_d("[%d] release.", _id);
}

void media::ffmpeg_mp4::reset_tmp_files() {
    std::remove(f_rgb_name.c_str());
    std::remove(f_264_name.c_str());
    std::remove(f_pcm_name.c_str());
    std::remove(f_aac_name.c_str());
    std::remove(name.c_str());
    fclose(fopen(f_rgb_name.c_str(), "wb+"));
    fclose(fopen(f_264_name.c_str(), "wb+"));
    fclose(fopen(f_pcm_name.c_str(), "wb+"));
    fclose(fopen(f_aac_name.c_str(), "wb+"));
    fclose(fopen(name.c_str(), "wb+"));
}

/*
 * run in media encode thread
 */
void media::ffmpeg_mp4::encode_frame(std::shared_ptr<image_frame> &&img_frame,
                                     std::shared_ptr<audio_frame> &&aud_frame) {
    if (img_frame != nullptr && img_frame->available()) {
        int32_t w, h; uint32_t *data;
        img_frame->get(&w, &h, &data);
        encode_image_frame(w, h, data);
    }
    if (aud_frame != nullptr && aud_frame->available()) {
        int32_t count; int8_t *data;
        aud_frame->get(&count, &data);
        encode_audio_frame();
    }
    ++pts;
}

void media::ffmpeg_mp4::request_stop() {
    req_stop = true;
    log_d("[%d] request stop.", _id);
}

/*
 * run in media encode thread
 */
bool media::ffmpeg_mp4::check_req_stop() {
    if (req_stop) {
        log_d("[%d] -check- request stoped.", _id);
    }
    return req_stop;
}

void media::ffmpeg_mp4::init_image_encode() {
    i_sws_ctx = sws_getContext(image.width, image.height, AV_PIX_FMT_ARGB, image.width, image.height,
            AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (i_sws_ctx == nullptr) {
        log_d("init_image_encode sws_getContext fail.");
        return;
    }

    int32_t res = avformat_alloc_output_context2(&if_ctx, nullptr, nullptr, f_264_name.c_str());
    if (res < 0) {
        log_e("init_image_encode avformat_alloc_output_context2 fail[%d].", res);
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        return;
    }

    AVOutputFormat *out_fmt = if_ctx->oformat;
    AVCodec *codec = avcodec_find_encoder(out_fmt->video_codec);
    if (codec == nullptr) {
        log_e("init_image_encode avcodec_find_encoder fail.");
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    if_ctx->video_codec = codec;
    ic_ctx = avcodec_alloc_context3(if_ctx->video_codec);
    if (ic_ctx == nullptr) {
        log_e("init_image_encode avcodec_alloc_context3 fail.");
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    ic_ctx->codec_id = out_fmt->video_codec;
    ic_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
    ic_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    ic_ctx->width = image.width;
    ic_ctx->height = image.height;
    ic_ctx->time_base = {1, image.fps<=0?15:(int32_t)image.fps};
    ic_ctx->bit_rate = 128000;
    ic_ctx->gop_size = 10;
    ic_ctx->qmin = 10;
    ic_ctx->qmax = 51;
    ic_ctx->max_b_frames = 1;

    i_stm = avformat_new_stream(if_ctx, if_ctx->video_codec);
    if (i_stm == nullptr) {
        log_e("init_image_encode avformat_new_stream fail.");
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    i_stm->id = (int32_t)if_ctx->nb_streams - 1;
    i_stm->time_base = {1, image.fps<=0?15:(int32_t)image.fps};
    res = avcodec_parameters_from_context(i_stm->codecpar, ic_ctx);
    if (res < 0) {
        log_e("init_image_encode avcodec_parameters_from_context fail.");
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    AVDictionary *options = nullptr;
    if (ic_ctx->codec_id == AV_CODEC_ID_H264) {
        av_dict_set(&options, "preset", "superfast", 0);
        av_dict_set(&options, "tune", "zerolatency", 0);
    }

    res = avcodec_open2(ic_ctx, if_ctx->video_codec, &options);
    if (res < 0) {
        log_e("init_image_encode avcodec_open2 fail.");
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    i_rgb_frm = av_frame_alloc();
    if (i_rgb_frm == nullptr) {
        log_e("init_image_encode av_frame_alloc fail.");
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    i_rgb_frm->format = AV_PIX_FMT_ARGB;
    i_rgb_frm->width = image.width;
    i_rgb_frm->height = image.height;

    res = av_frame_get_buffer(i_rgb_frm, 0);
    if (res < 0) {
        log_e("init_image_encode av_frame_get_buffer fail.");
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    res = av_frame_make_writable(i_rgb_frm);
    if (res < 0) {
        log_e("init_image_encode av_frame_make_writable fail.");
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    i_yuv_frm = av_frame_alloc();
    if (i_yuv_frm == nullptr) {
        log_e("init_image_encode av_frame_alloc fail.");
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    i_yuv_frm->format = AV_PIX_FMT_YUV420P;
    i_yuv_frm->width = image.width;
    i_yuv_frm->height = image.height;

    res = av_frame_get_buffer(i_yuv_frm, 0);
    if (res < 0) {
        log_e("init_image_encode av_frame_get_buffer fail.");
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        av_frame_free(&i_yuv_frm);
        i_yuv_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    res = av_frame_make_writable(i_yuv_frm);
    if (res < 0) {
        log_e("init_image_encode av_frame_make_writable fail.");
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        av_frame_free(&i_yuv_frm);
        i_yuv_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    log_d("init_image_encode success.");
}

void media::ffmpeg_mp4::init_audio_encode() {
    int32_t res = avformat_alloc_output_context2(&af_ctx, nullptr, nullptr, f_aac_name.c_str());
    if (res < 0) {
        log_e("init_audio_encode avformat_alloc_output_context2 fail[%d].", res);
        return;
    }

    AVOutputFormat *out_fmt = af_ctx->oformat;
    AVCodec *codec = avcodec_find_encoder(out_fmt->audio_codec);
    if (codec == nullptr) {
        log_e("init_audio_encode avcodec_find_encoder fail.");
        avformat_free_context(af_ctx);
        af_ctx = nullptr;
        return;
    }

    af_ctx->audio_codec = codec;
    ac_ctx = avcodec_alloc_context3(af_ctx->audio_codec);
    if (ac_ctx == nullptr) {
        log_e("init_audio_encode avcodec_alloc_context3 fail.");
        avformat_free_context(af_ctx);
        af_ctx = nullptr;
        return;
    }

    ac_ctx->bit_rate = 128000;
    ac_ctx->codec_id = out_fmt->audio_codec;
    ac_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
    ac_ctx->channels = audio.channels;
    ac_ctx->channel_layout = av_get_default_channel_layout(audio.channels);
    ac_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    ac_ctx->sample_rate = audio.sample_rate;

    a_stm = avformat_new_stream(af_ctx, af_ctx->audio_codec);
    if (a_stm == nullptr) {
        log_e("init_audio_encode avformat_new_stream fail.");
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        avformat_free_context(af_ctx);
        af_ctx = nullptr;
        return;
    }

    a_stm->id = (int32_t)af_ctx->nb_streams - 1;
    a_stm->time_base = {1, (int32_t)audio.sample_rate};
    res = avcodec_parameters_from_context(a_stm->codecpar, ac_ctx);
    if (res < 0) {
        log_e("init_audio_encode avcodec_parameters_from_context fail.");
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        avformat_free_context(af_ctx);
        af_ctx = nullptr;
        return;
    }

    res = avcodec_open2(ac_ctx, af_ctx->audio_codec, nullptr);
    if (res < 0) {
        log_e("init_audio_encode avcodec_open2 fail.");
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        avformat_free_context(af_ctx);
        af_ctx = nullptr;
        return;
    }

    a_frm = av_frame_alloc();
    if (a_frm == nullptr) {
        log_e("init_audio_encode av_frame_alloc fail.");
        avcodec_close(ac_ctx);
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        avformat_free_context(af_ctx);
        af_ctx = nullptr;
        return;
    }

    a_frm->format = ac_ctx->sample_fmt;
    a_frm->nb_samples = ac_ctx->frame_size;
    a_frm->channel_layout = ac_ctx->channel_layout;

    res = av_frame_get_buffer(a_frm, 0);
    if (res < 0) {
        log_e("init_audio_encode av_frame_get_buffer fail.");
        av_frame_free(&a_frm);
        a_frm = nullptr;
        avcodec_close(ac_ctx);
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        avformat_free_context(af_ctx);
        af_ctx = nullptr;
        return;
    }

    res = av_frame_make_writable(a_frm);
    if (res < 0) {
        log_e("init_audio_encode av_frame_make_writable fail.");
        av_frame_free(&a_frm);
        a_frm = nullptr;
        avcodec_close(ac_ctx);
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        avformat_free_context(af_ctx);
        af_ctx = nullptr;
        return;
    }

    log_d("init_audio_encode success. frame size:%d.", a_frm->linesize[0]);
}

void media::ffmpeg_mp4::close_image_encode() {
    if (i_sws_ctx) sws_freeContext(i_sws_ctx);
    i_sws_ctx = nullptr;
    if (i_rgb_frm) av_frame_free(&i_rgb_frm);
    i_rgb_frm = nullptr;
    if (i_yuv_frm) av_frame_free(&i_yuv_frm);
    i_yuv_frm = nullptr;
    if (ic_ctx) avcodec_close(ic_ctx);
    if (ic_ctx) avcodec_free_context(&ic_ctx);
    ic_ctx = nullptr;
    if (if_ctx) avformat_free_context(if_ctx);
    if_ctx = nullptr;
}

void media::ffmpeg_mp4::close_audio_encode() {
    if (a_frm) av_frame_free(&a_frm);
    a_frm = nullptr;
    if (ac_ctx) avcodec_close(ac_ctx);
    if (ac_ctx) avcodec_free_context(&ac_ctx);
    ac_ctx = nullptr;
    if (af_ctx) avformat_free_context(af_ctx);
    af_ctx = nullptr;
}

/*
 * run in media encode thread
 */
void media::ffmpeg_mp4::encode_image_frame(int32_t w, int32_t h, uint32_t *data) {
//    memcpy(i_rgb_frm->data[0], data, i_rgb_frm->linesize[0]);
//    int32_t res = sws_scale(i_sws_ctx, i_rgb_frm->data, i_rgb_frm->linesize,
//            0, h, i_yuv_frm->data, i_yuv_frm->linesize);
//    if (res <= 0) {
//        log_e("encode_image_frame sws_scale fail[%d].", res);
//        return;
//    }
//
//    i_yuv_frm->pts = pts;
//    res = avcodec_send_frame(ic_ctx, i_yuv_frm);
//    if (res < 0) {
////        char err[64];
////        av_strerror(res, err, 64);
////        log_e("encode_image_frame avcodec_send_frame fail[%d]%s.", res, err);
//        return;
//    }
//
////    log_d("encode_image_frame avcodec_send_frame success.");
//
//    while (true) {
//        AVPacket *pkt = av_packet_alloc();
//        if (pkt == nullptr) {
//            log_e("encode_image_frame av_packet_alloc fail.");
//            return;
//        }
//        av_init_packet(pkt);
//
//        res = avcodec_receive_packet(ic_ctx, pkt);
//        if (res < 0) {
//            av_packet_free(&pkt);
////            char err[64];
////            av_strerror(res, err, 64);
////            log_e("encode_image_frame avcodec_receive_packet fail[%d]%s.", res, err);
//            break;
//        }
//
////        log_d("encode_image_frame avcodec_receive_packet success.");
//
//        av_packet_free(&pkt);
//    }
}

/*
 * run in media loop thread
 */
void media::ffmpeg_mp4::encode_audio_frame() {
}
