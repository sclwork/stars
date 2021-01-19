//
// Created by scliang on 1/18/21.
//

#ifndef STARS_AUDIO_RECORDER_H
#define STARS_AUDIO_RECORDER_H

#include <cstdint>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "proc/audio_frame.h"

namespace media {

const int32_t PERIOD_TIME  = 10;  // 10ms
const int32_t PCM_BUF_SIZE = 320; // 320bytes

class audio_recorder {
public:
    audio_recorder(uint32_t channels = 2, uint32_t sample_rate = 44100);
    ~audio_recorder();

public:
    bool recording() const;
    bool start_record();
    void stop_record();
    std::shared_ptr<audio_frame> collect_frame();

private:
    audio_recorder(audio_recorder&&) = delete;
    audio_recorder(const audio_recorder&) = delete;
    audio_recorder& operator=(audio_recorder&&) = delete;
    audio_recorder& operator=(const audio_recorder&) = delete;

private:
    void init_objs();
    bool recordable() const;
    bool enqueue(bool chk_recording);

private:
    void handle_frame();

private:
    static void queue_callback(SLAndroidSimpleBufferQueueItf queue, void *ctx);

private:
    SLObjectItf eng_obj;
    SLEngineItf eng_eng;
    SLObjectItf rec_obj;
    SLRecordItf rec_eng;
    SLAndroidSimpleBufferQueueItf rec_queue;
    //////////////////////////////////////////
    uint32_t channels;
    SLuint32 sampling_rate;
    uint32_t sample_rate;
    //////////////////////////////////////////
    int8_t  *pcm_data;
    int16_t *sht_data;
    //////////////////////////////////////////
    std::shared_ptr<audio_frame> cache;
    std::shared_ptr<audio_frame> frame;
};

} //namespace media

#endif //STARS_AUDIO_RECORDER_H
