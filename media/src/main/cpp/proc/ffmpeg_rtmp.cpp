//
// Created by Scliang on 3/8/21.
//

#include "proc.h"
#include "jni_log.h"
#include "ffmpeg_rtmp.h"

#define log_d(...)  LOG_D("Media-Native:ffmpeg_rtmp", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:ffmpeg_rtmp", __VA_ARGS__)

namespace media {
} //namespace media

media::ffmpeg_rtmp::ffmpeg_rtmp(int32_t id, std::string &&n, image_args &&img, audio_args &&aud)
:_id(id), i_pts(0), a_pts(0), a_encode_offset(0), a_encode_length(0), name(n), image(img), audio(aud),
vf_ctx(nullptr), ic_ctx(nullptr), i_stm(nullptr), i_sws_ctx(nullptr), i_rgb_frm(nullptr), i_yuv_frm(nullptr),
ac_ctx(nullptr), a_stm(nullptr), a_swr_ctx(nullptr), a_frm(nullptr), a_encode_cache(nullptr) {
    image.update_frame_size();
    log_d("[%d] created. [v:%d,%d,%d,%d],[a:%d,%d,%d].",
          _id, image.width, image.height, image.channels, image.frame_size,
          audio.channels, audio.sample_rate, audio.frame_size);
}

media::ffmpeg_rtmp::~ffmpeg_rtmp() {
    if (i_rgb_frm) av_frame_free(&i_rgb_frm);
    if (i_yuv_frm) av_frame_free(&i_yuv_frm);
    if (i_sws_ctx) sws_freeContext(i_sws_ctx);
    if (ic_ctx) avcodec_close(ic_ctx);
    if (ic_ctx) avcodec_free_context(&ic_ctx);
    if (a_frm) av_frame_free(&a_frm);
    if (a_swr_ctx) swr_free(&a_swr_ctx);
    if (ac_ctx) avcodec_close(ac_ctx);
    if (ac_ctx) avcodec_free_context(&ac_ctx);
    if (vf_ctx) avformat_free_context(vf_ctx);
    log_d("[%d] release.", _id);
}

void media::ffmpeg_rtmp::init() {
    av_register_all();
    avcodec_register_all();
    avformat_network_init();
    // init image encode
    const char *rtmp_url = name.c_str();

    AVCodec *i_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (i_codec == nullptr) {
        log_e("init_image_encode avcodec_find_encoder fail.");
        return;
    }

    log_d("video_codec: %s.", i_codec->long_name);
    ic_ctx = avcodec_alloc_context3(i_codec);
    if (ic_ctx == nullptr) {
        log_e("init_image_encode avcodec_alloc_context3 fail.");
        return;
    }

    ic_ctx->codec_id = i_codec->id;
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

    int32_t res = avcodec_open2(ic_ctx, i_codec, &options);
    if (res < 0) {
        log_e("init_image_encode avcodec_open2 fail.");
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        return;
    }

    res = avformat_alloc_output_context2(&vf_ctx, nullptr, "flv", rtmp_url);
    if (res < 0) {
        log_e("init_image_encode avformat_alloc_output_context2 fail[%d].", res);
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        return;
    }

    log_d("init_image_encode vf_ctx stream num: %d.", vf_ctx->nb_streams);

    i_stm = avformat_new_stream(vf_ctx, i_codec);
    if (i_stm == nullptr) {
        log_e("init_image_encode avformat_new_stream fail.");
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        return;
    }

    i_stm->id = 0;
    i_stm->time_base = {1, image.fps<=0?15:(int32_t)image.fps};
    i_stm->codec->codec_tag = 0;
	if (vf_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
		i_stm->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    res = avio_open(&vf_ctx->pb, rtmp_url, AVIO_FLAG_WRITE);
    if (res < 0) {
        char err[64];
        av_strerror(res, err, 64);
        log_e("init_image_encode avio_open fail: (%d) %s", res, err);
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        return;
    }

    res = avcodec_parameters_from_context(i_stm->codecpar, ic_ctx);
    if (res < 0) {
        log_e("init_image_encode avcodec_parameters_from_context fail.");
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        return;
    }

    i_sws_ctx = sws_getContext(image.width, image.height, AV_PIX_FMT_RGBA, image.width, image.height,
            AV_PIX_FMT_YUV420P, SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (i_sws_ctx == nullptr) {
        log_e("init_image_encode sws_getContext fail.");
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
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
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        return;
    }

    i_rgb_frm->format = AV_PIX_FMT_RGBA;
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
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
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
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
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
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
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
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
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
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        return;
    }

    res = avformat_write_header(vf_ctx, nullptr);
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
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        return;
    }

    i_pts = 0;
    log_d("init_image_encode success.");

    // init audio encode
    AVCodec *a_codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (a_codec == nullptr) {
        log_e("init_audio_encode avcodec_find_encoder fail.");
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        av_frame_free(&i_yuv_frm);
        i_yuv_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        return;
    }

    log_d("audio_codec: %s.", a_codec->long_name);
    ac_ctx = avcodec_alloc_context3(a_codec);
    if (ac_ctx == nullptr) {
        log_e("init_audio_encode avcodec_alloc_context3 fail.");
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        av_frame_free(&i_yuv_frm);
        i_yuv_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        return;
    }

    ac_ctx->bit_rate = 128000;
    ac_ctx->sample_rate = audio.sample_rate;
    ac_ctx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    ac_ctx->channels = audio.channels;
    ac_ctx->channel_layout = av_get_default_channel_layout(audio.channels);
    ac_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    res = avcodec_open2(ac_ctx, a_codec, nullptr);
    if (res < 0) {
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        av_frame_free(&i_yuv_frm);
        i_yuv_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        log_e("init_audio_encode avcodec_open2 fail: %d.", res);
        return;
    }

    a_stm = avformat_new_stream(vf_ctx, a_codec);
    if (a_stm == nullptr) {
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        av_frame_free(&i_yuv_frm);
        i_yuv_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        log_e("init_audio_encode avformat_new_stream fail.");
        return;
    }

    a_stm->id = 1;
    a_stm->time_base = {1, (int32_t)audio.sample_rate };
    res = avcodec_parameters_from_context(a_stm->codecpar, ac_ctx);
    if (res < 0) {
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        av_frame_free(&i_yuv_frm);
        i_yuv_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        log_e("init_audio_encode avcodec_parameters_from_context fail: %d.", res);
        return;
    }

    a_frm = av_frame_alloc();
    if (a_frm == nullptr) {
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        av_frame_free(&i_yuv_frm);
        i_yuv_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        log_e("init_audio_encode av_frame_alloc fail.");
        return;
    }

    int32_t nb_samples;
    if (ac_ctx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) {
        nb_samples = 1024;
    } else {
        nb_samples = ac_ctx->frame_size;
    }
    log_d("init_audio_encode nb samples: %d", nb_samples);
    a_frm->nb_samples = nb_samples;
    a_frm->format = ac_ctx->sample_fmt;
    a_frm->channel_layout = ac_ctx->channel_layout;

    res = av_frame_get_buffer(a_frm, 0);
    if (res < 0) {
        av_frame_free(&a_frm);
        a_frm = nullptr;
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        av_frame_free(&i_yuv_frm);
        i_yuv_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        log_e("init_audio_encode av_frame_get_buffer fail: %d.", res);
        return;
    }

    res = av_frame_make_writable(a_frm);
    if (res < 0) {
        av_frame_free(&a_frm);
        a_frm = nullptr;
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        av_frame_free(&i_yuv_frm);
        i_yuv_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        log_e("init_audio_encode av_frame_make_writable fail: %d.", res);
        return;
    }

    a_swr_ctx = swr_alloc_set_opts(nullptr, a_frm->channel_layout,
            ac_ctx->sample_fmt, audio.sample_rate, a_frm->channel_layout,
            AV_SAMPLE_FMT_S16, audio.sample_rate, 0, nullptr);
    if (a_swr_ctx == nullptr) {
        av_frame_free(&a_frm);
        a_frm = nullptr;
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        av_frame_free(&i_yuv_frm);
        i_yuv_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        log_e("init_audio_encode swr_alloc_set_opts fail.");
        return;
    }

    res = swr_init(a_swr_ctx);
    if (res < 0) {
        char err[64];
        av_strerror(res, err, 64);
        log_e("init_audio_encode swr_init fail: (%d) %s", res, err);
        swr_free(&a_swr_ctx);
        a_swr_ctx = nullptr;
        av_frame_free(&a_frm);
        a_frm = nullptr;
        avcodec_free_context(&ac_ctx);
        ac_ctx = nullptr;
        av_frame_free(&i_rgb_frm);
        i_rgb_frm = nullptr;
        av_frame_free(&i_yuv_frm);
        i_yuv_frm = nullptr;
        sws_freeContext(i_sws_ctx);
        i_sws_ctx = nullptr;
        avcodec_close(ic_ctx);
        avcodec_free_context(&ic_ctx);
        ic_ctx = nullptr;
        avformat_free_context(vf_ctx);
        vf_ctx = nullptr;
        return;
    } else {
        log_d("init_audio_encode swr_init OK - channel:%d, channelLayout:%lld, sampleRate:%d",
             audio.channels, a_frm->channel_layout, audio.sample_rate);
    }

    a_pts = 0;
    a_encode_offset = 0;
    a_encode_length = av_samples_get_buffer_size(nullptr, ac_ctx->channels, ac_ctx->frame_size, ac_ctx->sample_fmt, 1);
    a_encode_cache = (int8_t *) malloc(sizeof(int8_t) * a_encode_length);
    log_d("init_audio_encode success. frame size:%d.", a_encode_length);
}

void media::ffmpeg_rtmp::complete() {
    // close rtmp stream
    if (vf_ctx) {
        av_write_trailer(vf_ctx);
        avio_closep(&vf_ctx->pb);
    }

    // close image encode
    if (i_sws_ctx) sws_freeContext(i_sws_ctx);
    i_sws_ctx = nullptr;
    if (i_rgb_frm) av_frame_free(&i_rgb_frm);
    i_rgb_frm = nullptr;
    if (i_yuv_frm) av_frame_free(&i_yuv_frm);
    i_yuv_frm = nullptr;
    if (ic_ctx) avcodec_close(ic_ctx);
    if (ic_ctx) avcodec_free_context(&ic_ctx);
    ic_ctx = nullptr;
    
    // close audio encode
    if (a_swr_ctx) swr_free(&a_swr_ctx);
    a_swr_ctx = nullptr;
    if (a_frm) av_frame_free(&a_frm);
    a_frm = nullptr;
    if (ac_ctx) avcodec_close(ac_ctx);
    if (ac_ctx) avcodec_free_context(&ac_ctx);
    ac_ctx = nullptr;

    // close video format ctx
    if (vf_ctx) avformat_free_context(vf_ctx);
    vf_ctx = nullptr;
}

void media::ffmpeg_rtmp::encode_frame(std::shared_ptr<image_frame> &&img_frame,
                                      std::shared_ptr<audio_frame> &&aud_frame) {
    int32_t w = 0, h = 0, count = 0; uint32_t *img_data = nullptr; int8_t *aud_data = nullptr;
    if (img_frame != nullptr && img_frame->available()) {
        img_frame->get(&w, &h, &img_data);
    }
    if (aud_frame != nullptr && aud_frame->available()) {
        aud_frame->get(&count, &aud_data);
    }
    encode_ia_frame(w, h, img_data, count, aud_data);
}

void media::ffmpeg_rtmp::encode_ia_frame(int32_t w, int32_t h, const uint32_t* const img_data,
                                         int32_t count, const int8_t* const aud_data) {
    // encode image
    if (w > 0 && h > 0 && img_data != nullptr) {
        avpicture_fill((AVPicture *)i_rgb_frm, (uint8_t *)img_data, AV_PIX_FMT_RGBA, w, h);
        int32_t res = sws_scale(i_sws_ctx, i_rgb_frm->data, i_rgb_frm->linesize,
                0, h, i_yuv_frm->data, i_yuv_frm->linesize);
        if (res <= 0) {
            char err[64];
            av_strerror(res, err, 64);
            log_e("encode_image_frame sws_scale fail[%d] %s.", res, err);
            return;
        }

        i_yuv_frm->pts = i_pts++ * (i_stm->time_base.den) / ((i_stm->time_base.num) * 25);
        res = avcodec_send_frame(ic_ctx, i_yuv_frm);
        if (res < 0) {
        //    char err[64];
        //    av_strerror(res, err, 64);
        //    log_e("encode_image_frame avcodec_send_frame fail[%d]%s.", res, err);
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
            //    char err[64];
            //    av_strerror(res, err, 64);
            //    log_e("encode_image_frame avcodec_receive_packet fail[%d]%s.", res, err);
                break;
            }

            if (vf_ctx->nb_streams > 0) {
                av_packet_rescale_ts(pkt, ic_ctx->time_base, vf_ctx->streams[0]->time_base);
            }
            pkt->stream_index = 0;
        //    log_d("encode_image_frame avcodec_receive_packet success.");

            av_interleaved_write_frame(vf_ctx, pkt);
            av_packet_free(&pkt);
        }
    }
    
    // encode audio
    if (count > 0 && aud_data != nullptr) {
        // log_d("encode_audio_frame remain: %d, count: %d.", a_encode_offset, count);
        int32_t off = 0;
        int32_t frm_size = a_encode_length;
        while(true) {
            if (count - off >= frm_size) {
                if (a_encode_offset > 0) {
                    memcpy(a_encode_cache + a_encode_offset, aud_data + off,
                            sizeof(int8_t) * (frm_size - a_encode_offset));
                    off += frm_size - a_encode_offset;
                    a_encode_offset = 0;
                } else {
                    memcpy(a_encode_cache, aud_data + off, sizeof(int8_t) * frm_size);
                    off += frm_size;
                }
                // log_d("encode_audio_frame start swr_convert.");
                u_char *pData[1] = {nullptr};
                pData[0] = (u_char *) a_encode_cache;
                if (swr_convert(a_swr_ctx, a_frm->data, swr_get_out_samples(a_swr_ctx, a_frm->nb_samples),
                        (const uint8_t **)pData, a_frm->nb_samples) >= 0) {
                    // log_d("encode_audio_frame swr_convert success.");
                    a_frm->pts = a_pts++;
                    // log_d("encode_audio_frame start avcodec_send_frame.");
                    if (avcodec_send_frame(ac_ctx, a_frm) >= 0) {
                        while (true) {
                            AVPacket *pkt = av_packet_alloc();
                            if (pkt == nullptr) {
                                log_e("encode_audio_frame av_packet_alloc fail.");
                                break;
                            }
                            av_init_packet(pkt);

                            int32_t res = avcodec_receive_packet(ac_ctx, pkt);
                            if (res < 0) {
                                av_packet_free(&pkt);
                                break;
                            }

                            if (vf_ctx->nb_streams > 1) {
                                av_packet_rescale_ts(pkt, ac_ctx->time_base, vf_ctx->streams[1]->time_base);
                            }
                            pkt->stream_index = 1;
                            // log_d("encode_audio_frame avcodec_receive_packet success.");

                            // TODO: rtmp audio frame
                            // av_interleaved_write_frame(vf_ctx, pkt);
                            av_packet_free(&pkt);
                        }
                    }
                }
            } else {
                a_encode_offset = count - off;
                memcpy(a_encode_cache, aud_data + off, sizeof(int8_t) * a_encode_offset);
                break;
            }
        }
    }
}
