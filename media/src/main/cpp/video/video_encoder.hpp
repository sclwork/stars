//
// Created by Scliang on 3/23/21.
//

#ifndef STARS_VIDEO_ENCODER_HPP
#define STARS_VIDEO_ENCODER_HPP

#include <ctime>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "jni_log.h"
#include "video_recorder.h"
#include "image/collect/image_recorder.h"
#include "audio/collect/audio_recorder.h"
#include "proc/proc.h"
#include "proc/mnn.h"
#include "proc/opencv.h"
#include "proc/ffmpeg_mp4.h"
#include "proc/ffmpeg_rtmp.h"
#include "proc/ffmpeg_loudnorm.h"
#include "proc/webrtc_ns.h"

#define log_d_(...)  LOG_D("Media-Native:video_recorder", __VA_ARGS__)
#define log_e_(...)  LOG_E("Media-Native:video_recorder", __VA_ARGS__)

namespace media {

class video_encoder {
public:
    video_encoder(
            bool use_loudnorm,
            int32_t ns_mode,
            std::string &&file_root,
            std::string &&name,
            image_args &&img_args, audio_args &&aud_args,
            std::shared_ptr<std::atomic_bool> &runnable,
            std::shared_ptr<moodycamel::ConcurrentQueue<frame>> &&fQ)
     :_mux(), runnable(runnable),
      loudnorm(use_loudnorm?std::make_shared<ffmpeg_loudnorm>("I=-16:tp=-1.5:LRA=11",
                                                              std::forward<audio_args>(aud_args)):nullptr),
      ns(ns_mode>=0?std::make_shared<webrtc_ns>(ns_mode, aud_args.sample_rate):nullptr),
      mp4(endWith(name,".mp4")?std::make_shared<ffmpeg_mp4>(
              std::forward<std::string>(name),
              std::forward<image_args>(img_args), std::forward<audio_args>(aud_args)):nullptr),
      rtmp(startWith(name,"rtmp")?std::make_shared<ffmpeg_rtmp>(
              std::forward<std::string>(file_root), std::forward<std::string>(name),
              std::forward<image_args>(img_args), std::forward<audio_args>(aud_args)):nullptr),
      frameQ(fQ) {
        if (loudnorm != nullptr) loudnorm->init();
        if (ns != nullptr) ns->init();
        if (mp4 != nullptr) mp4->init();
        if (rtmp != nullptr) rtmp->init();
        log_d_("video_encoder created.");
    }

    ~video_encoder() {
        if (mp4 != nullptr) mp4->complete();
        if (rtmp != nullptr) rtmp->complete();
        if (ns != nullptr) ns->complete();
        if (loudnorm != nullptr) loudnorm->complete();
        log_d_("video_encoder release.");
    }

public:
    bool running() const {
        std::lock_guard<std::mutex> lg(_mux);
        return runnable != nullptr && *runnable;
    }

    void unrunning() {
        std::lock_guard<std::mutex> lg(_mux);
        if (runnable != nullptr) {
            *(runnable) = false;
        }
    }

    void run() {
        if (frameQ != nullptr) {
            check_frameQ();
            check_loudnorm();
            check_ns();
        }
    }

private:
    void check_frameQ() {
        frame frm;
        if (frameQ->try_dequeue(frm)) {
            if (loudnorm != nullptr) {
                loudnorm->encode_frame(frm.audio);
                if (mp4 != nullptr) {
                    mp4->encode_image_frame(frm.image);
                }
                if (rtmp != nullptr) {
                    rtmp->encode_image_frame(frm.image);
                }
            } else {
                if (ns != nullptr) {
                    ns->encode_frame(frm.audio);
                    if (mp4 != nullptr) {
                        mp4->encode_image_frame(frm.image);
                    }
                    if (rtmp != nullptr) {
                        rtmp->encode_image_frame(frm.image);
                    }
                } else {
                    if (mp4 != nullptr) {
                        mp4->encode_image_frame(frm.image);
                        mp4->encode_audio_frame(frm.audio);
                    }
                    if (rtmp != nullptr) {
                        rtmp->encode_image_frame(frm.image);
                        rtmp->encode_audio_frame(frm.audio);
                    }
                }
            }
        }
    }

    void check_loudnorm() {
        if (loudnorm != nullptr) {
            std::shared_ptr<media::audio_frame> audio = loudnorm->get_encoded_frame();
            if (audio != nullptr) {
                if (ns != nullptr) {
                    ns->encode_frame(audio);
                } else {
                    if (mp4 != nullptr) {
                        mp4->encode_audio_frame(audio);
                    }
                    if (rtmp != nullptr) {
                        rtmp->encode_audio_frame(audio);
                    }
                }
            }
        }
    }

    void check_ns() {
        if (ns != nullptr) {
            std::shared_ptr<media::audio_frame> audio = ns->get_encoded_frame();
            if (audio != nullptr) {
                if (mp4 != nullptr) {
                    mp4->encode_audio_frame(audio);
                }
                if (rtmp != nullptr) {
                    rtmp->encode_audio_frame(audio);
                }
            }
        }
    }

private:
    mutable std::mutex _mux;
    std::shared_ptr<std::atomic_bool> runnable;
    std::shared_ptr<ffmpeg_loudnorm> loudnorm;
    std::shared_ptr<webrtc_ns> ns;
    std::shared_ptr<ffmpeg_mp4> mp4;
    std::shared_ptr<ffmpeg_rtmp> rtmp;
    std::shared_ptr<moodycamel::ConcurrentQueue<frame>> frameQ;
};

} //namespace media

#endif //STARS_VIDEO_ENCODER_HPP
