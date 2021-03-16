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

// PCM Size=采样率*采样时间*采样位深/8*通道数（Bytes）
const int32_t PERIOD_TIME  = 10;  // 10ms
const int32_t PCM_BUF_SIZE = 320; // 320bytes

class audio_recorder {
public:
    audio_recorder(uint32_t channels = 2, uint32_t sample_rate = 44100);
    ~audio_recorder();

public:
    /**
     * @return true: audio recorder recording
     */
    bool recording() const;
    /**
     * start audio record
     * @return true: start success
     */
    bool start_record();
    /**
     * stop audio record
     */
    void stop_record();

public:
    /**
     * @return pcm channels num
     */
    uint32_t get_channels() const;
    /**
     * @return pcm sample rate
     */
    uint32_t get_sample_rate() const;
    /**
     * @return audio frame data size
     */
    uint32_t get_frame_size() const;
    /**
     * collect an audio frame
     * @param changed true: audio frame data cache changed
     * @return collect success audio_frame, all return audio_frame is same address
     */
    std::shared_ptr<audio_frame> collect_frame(bool *changed);

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
    uint8_t *pcm_data;
    //////////////////////////////////////////
    uint32_t                     frm_size;
    std::atomic_bool             frm_changed;
    std::shared_ptr<audio_frame> cache;
    std::shared_ptr<audio_frame> frame;
};

} //namespace media

#endif //STARS_AUDIO_RECORDER_H
