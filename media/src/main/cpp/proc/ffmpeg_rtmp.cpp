//
// Created by Scliang on 3/8/21.
//

#include "jni_log.h"
#include "ffmpeg_rtmp.h"

#define log_d(...)  LOG_D("Media-Native:ffmpeg_rtmp", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:ffmpeg_rtmp", __VA_ARGS__)

namespace media {
} //namespace media

media::ffmpeg_rtmp::ffmpeg_rtmp(int32_t id, std::string &&n, image_args &&img, audio_args &&aud)
:_id(id), i_pts(0), st_time(-1), name(n), image(img), audio(aud),
ic_ctx(nullptr), if_ctx(nullptr), i_stm(nullptr), i_sws_ctx(nullptr), i_rgb_frm(nullptr), i_yuv_frm(nullptr) {
    image.update_frame_size();
    log_d("[%d] created. [v:%d,%d,%d,%d],[a:%d,%d,%d].",
          _id, image.width, image.height, image.channels, image.frame_size,
          audio.channels, audio.sample_rate, audio.frame_size);
}

media::ffmpeg_rtmp::~ffmpeg_rtmp() {
    if (i_rgb_frm) av_frame_free(&i_rgb_frm);
    if (i_yuv_frm) av_frame_free(&i_yuv_frm);
    if (i_sws_ctx) sws_freeContext(i_sws_ctx);
    if (if_ctx) avformat_free_context(if_ctx);
    if (ic_ctx) avcodec_close(ic_ctx);
    if (ic_ctx) avcodec_free_context(&ic_ctx);
    log_d("[%d] release.", _id);
}

void media::ffmpeg_rtmp::init() {
    av_register_all();
    avcodec_register_all();
    avformat_network_init();
    init_image_encode();
}

void media::ffmpeg_rtmp::complete() {
    close_image_encode();
}

void media::ffmpeg_rtmp::encode_frame(std::shared_ptr<image_frame> &&img_frame,
                                      std::shared_ptr<audio_frame> &&aud_frame) {
    if (img_frame != nullptr && img_frame->available()) {
        int32_t w, h; uint32_t *data;
        img_frame->get(&w, &h, &data);
        encode_image_frame(w, h, data);
    }
}

void media::ffmpeg_rtmp::init_image_encode() {
    const char *rtmp_url = name.c_str();

    AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (codec == nullptr) {
        log_e("init_image_encode avcodec_find_encoder fail.");
        return;
    }

    log_d("video_codec: %s.", codec->long_name);
    ic_ctx = avcodec_alloc_context3(codec);
    if (ic_ctx == nullptr) {
        log_e("init_image_encode avcodec_alloc_context3 fail.");
        return;
    }

    ic_ctx->codec_id = codec->id;
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
    ic_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    AVDictionary *options = nullptr;
    if (ic_ctx->codec_id == AV_CODEC_ID_H264) {
        av_dict_set(&options, "preset", "superfast", 0);
        av_dict_set(&options, "tune", "zerolatency", 0);
    }

    int32_t res = avcodec_open2(ic_ctx, codec, &options);
    if (res < 0) {
        log_e("init_image_encode avcodec_open2 fail.");
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        return;
    }

    res = avformat_alloc_output_context2(&if_ctx, nullptr, "flv", rtmp_url);
    if (res < 0) {
        log_e("init_image_encode avformat_alloc_output_context2 fail[%d].", res);
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        return;
    }

    i_stm = avformat_new_stream(if_ctx, codec);
    if (i_stm == nullptr) {
        log_e("init_image_encode avformat_new_stream fail.");
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    // i_stm->id = (int32_t)if_ctx->nb_streams - 1;
    i_stm->time_base = {1, image.fps<=0?15:(int32_t)image.fps};
    i_stm->codec->codec_tag = 0;
	if (if_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
		i_stm->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    res = avio_open(&if_ctx->pb, rtmp_url, AVIO_FLAG_WRITE);
    if (res < 0) {
        char err[64];
        av_strerror(res, err, 64);
        log_e("init_image_encode avio_open fail: (%d) %s", res, err);
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    res = avcodec_parameters_from_context(i_stm->codecpar, ic_ctx);
    if (res < 0) {
        log_e("init_image_encode avcodec_parameters_from_context fail.");
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(if_ctx);
        if_ctx = nullptr;
        return;
    }

    i_sws_ctx = sws_getContext(image.width, image.height, AV_PIX_FMT_ARGB, image.width, image.height,
            AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (i_sws_ctx == nullptr) {
        log_e("init_image_encode sws_getContext fail.");
        avcodec_close(ic_ctx);
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

    res = avformat_write_header(if_ctx, nullptr);
    if (res < 0) {
        log_e("init_image_encode avformat_write_header fail.");
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

    i_pts = 0;
    log_d("init_image_encode success.");
}

void media::ffmpeg_rtmp::close_image_encode() {
    if (if_ctx) {
        av_write_trailer(if_ctx);
        avio_closep(&if_ctx->pb);
    }
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

void media::ffmpeg_rtmp::encode_image_frame(int32_t w, int32_t h, const uint32_t* const data) {
    if (st_time < 0) { st_time = av_gettime(); }
    log_d("encode_image_frame start_time: %d.", st_time);

    memcpy(i_rgb_frm->data[0], data, sizeof(uint8_t) * i_rgb_frm->linesize[0]);
    int32_t res = sws_scale(i_sws_ctx, i_rgb_frm->data, i_rgb_frm->linesize,
            0, h, i_yuv_frm->data, i_yuv_frm->linesize);
    if (res <= 0) {
        log_e("encode_image_frame sws_scale fail[%d].", res);
        return;
    }

    i_yuv_frm->pts = i_pts++ * (i_stm->time_base.den) / ((i_stm->time_base.num) * 25);
    res = avcodec_send_frame(ic_ctx, i_yuv_frm);
    if (res < 0) {
//        char err[64];
//        av_strerror(res, err, 64);
//        log_e("encode_image_frame avcodec_send_frame fail[%d]%s.", res, err);
        return;
    }

//    log_d("encode_image_frame avcodec_send_frame success.");

    while (true) {
        AVPacket *pkt = av_packet_alloc();
        if (pkt == nullptr) {
            log_e("encode_image_frame av_packet_alloc fail.");
            return;
        }
        av_init_packet(pkt);

        res = avcodec_receive_packet(ic_ctx, pkt);
        if (res < 0) {
            av_packet_free(&pkt);
//            char err[64];
//            av_strerror(res, err, 64);
//            log_e("encode_image_frame avcodec_receive_packet fail[%d]%s.", res, err);
            break;
        }

//        log_d("encode_image_frame avcodec_receive_packet success.");

        av_write_frame(if_ctx, pkt);
        av_packet_free(&pkt);
    }

    i_pts++;
}
