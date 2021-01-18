//
// Created by scliang on 1/18/21.
//

#ifndef STARS_AUDIO_RECORDER_H
#define STARS_AUDIO_RECORDER_H

#include <cstdint>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace media {

const int32_t PERIOD_TIME  = 0;
const int32_t PCM_BUF_SIZE = 320;

class audio_recorder {
public:
    audio_recorder(uint32_t channels = 2, uint32_t sample_rate = 44100);
    ~audio_recorder();

public:
    bool recording() const;
    bool start_record();
    void stop_record();

private:
    audio_recorder(audio_recorder&&) = delete;
    audio_recorder(const audio_recorder&) = delete;
    audio_recorder& operator=(audio_recorder&&) = delete;
    audio_recorder& operator=(const audio_recorder&) = delete;

private:
    void init_objs();
    bool recordable() const;
    bool enqueue(bool chk_recording);
    void handle_pcm();

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
    int8_t *pcm_data;
    int16_t *pcm_sdata;
};

} //namespace media

#endif //STARS_AUDIO_RECORDER_H
