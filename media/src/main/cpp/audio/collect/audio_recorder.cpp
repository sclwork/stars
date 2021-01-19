//
// Created by scliang on 1/18/21.
//

#include <string>
#include "log.h"
#include "audio_recorder.h"

#define log_d(...)  LOG_D("Media-Native:audio_recorder", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:audio_recorder", __VA_ARGS__)

namespace media {
} //namespace media

media::audio_recorder::audio_recorder(uint32_t channels, uint32_t sample_rate)
:eng_obj(nullptr), eng_eng(nullptr), rec_obj(nullptr), rec_eng(nullptr), rec_queue(nullptr),
channels(channels<=1?1:2), sampling_rate(sample_rate==44100?SL_SAMPLINGRATE_44_1:SL_SAMPLINGRATE_16),
sample_rate(sampling_rate / 1000),
pcm_data((int8_t*)malloc(sizeof(int8_t)*PCM_BUF_SIZE)),
sht_data((int16_t*)malloc(sizeof(int16_t)*(PCM_BUF_SIZE/2))),
cache(std::make_shared<audio_frame>(PCM_BUF_SIZE*1024)),
frame(std::make_shared<audio_frame>(PCM_BUF_SIZE*1024)) {
    log_d("created. channels:%d, sample_rate:%d.", this->channels, this->sample_rate);
    init_objs();
}

media::audio_recorder::~audio_recorder() {
    if (rec_obj) {
        (*rec_obj)->Destroy(rec_obj);
        rec_obj = nullptr;
    }
    if (eng_obj) {
        (*eng_obj)->Destroy(eng_obj);
        eng_obj = nullptr;
    }
    if (pcm_data) {
        free(pcm_data);
        pcm_data = nullptr;
    }
    if (sht_data) {
        free(sht_data);
        sht_data = nullptr;
    }
    log_d("release.");
}

bool media::audio_recorder::recordable() const {
    return rec_obj   != nullptr &&
           rec_eng   != nullptr &&
           rec_queue != nullptr;
}

bool media::audio_recorder::recording() const {
    if (!recordable()) {
        return false;
    }
    SLuint32 state;
    SLresult res = (*rec_eng)->GetRecordState(rec_eng, &state);
    return res == SL_RESULT_SUCCESS && state == SL_RECORDSTATE_RECORDING;
}

bool media::audio_recorder::enqueue(bool chk_recording) {
    if (chk_recording && !recording()) {
        return false;
    }
    SLresult res = (*rec_queue)->Enqueue(rec_queue, pcm_data, PCM_BUF_SIZE);
    return res == SL_RESULT_SUCCESS;
}

void media::audio_recorder::handle_frame() {
    if (cache != nullptr) {
        if (cache->cp_offset + PCM_BUF_SIZE > cache->size) {
            cache->cp_offset = 0;
        }
        memcpy(cache->cache + cache->cp_offset, pcm_data, sizeof(int8_t)*PCM_BUF_SIZE);
        cache->cp_offset += PCM_BUF_SIZE;
        if (frame != nullptr && cache->cp_offset >= cache->size) {
            memcpy(frame->cache, cache->cache, sizeof(int8_t)*frame->size);
        }
    }
}

bool media::audio_recorder::start_record() {
    if (!recordable()) {
        return false;
    }
    if (!enqueue(false)) {
        return false;
    }
    SLresult res = (*rec_eng)->SetRecordState(rec_eng, SL_RECORDSTATE_RECORDING);
    if (res != SL_RESULT_SUCCESS) {
        return false;
    }
    log_d("start record.");
    return true;
}

void media::audio_recorder::stop_record() {
    if (!recording()) {
        return;
    }
    (*rec_eng)->SetRecordState(rec_eng, SL_RECORDSTATE_STOPPED);
    log_d("stop record.");
}

std::shared_ptr<media::audio_frame> media::audio_recorder::collect_frame() {
    return frame;
}

void media::audio_recorder::init_objs() {
    SLresult res;
    SLmillisecond period;
    res = slCreateEngine(&eng_obj, 0, nullptr, 0, nullptr, nullptr);
    if (res != SL_RESULT_SUCCESS) {
        log_e("create eng obj fail. %d.", res);
        return;
    }
    res = (*eng_obj)->Realize(eng_obj, SL_BOOLEAN_FALSE);
    if (res != SL_RESULT_SUCCESS) {
        (*eng_obj)->Destroy(eng_obj);
        eng_obj = nullptr;
        log_e("realize eng obj fail. %d.", res);
        return;
    }
    res = (*eng_obj)->GetInterface(eng_obj, SL_IID_ENGINE, &eng_eng);
    if (res != SL_RESULT_SUCCESS) {
        (*eng_obj)->Destroy(eng_obj);
        eng_obj = nullptr;
        log_e("get eng eng fail. %d.", res);
        return;
    }
    SLDataLocator_IODevice ioDevice = {
            SL_DATALOCATOR_IODEVICE,
            SL_IODEVICE_AUDIOINPUT,
            SL_DEFAULTDEVICEID_AUDIOINPUT,
            nullptr
    };
    SLDataSource dataSrc = { &ioDevice, nullptr };
    SLDataLocator_AndroidSimpleBufferQueue bufferQueue = {
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 20 };
    SLDataFormat_PCM formatPcm = {
            SL_DATAFORMAT_PCM, channels, sampling_rate,
            SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
            channels==1?SL_SPEAKER_FRONT_CENTER:SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSink audioSink = { &bufferQueue, &formatPcm };
    const SLInterfaceID iid[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_ANDROIDCONFIGURATION };
    const SLboolean req[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
    res = (*eng_eng)->CreateAudioRecorder(eng_eng, &rec_obj, &dataSrc, &audioSink, 2, iid, req);
    if (res != SL_RESULT_SUCCESS) {
        (*eng_obj)->Destroy(eng_obj);
        eng_obj = nullptr;
        eng_eng = nullptr;
        log_e("create audio recorder fail. %d.", res);
        return;
    }
    res = (*rec_obj)->Realize(rec_obj, SL_BOOLEAN_FALSE);
    if (res != SL_RESULT_SUCCESS) {
        (*eng_obj)->Destroy(eng_obj);
        eng_obj = nullptr;
        eng_eng = nullptr;
        (*rec_obj)->Destroy(rec_obj);
        rec_obj = nullptr;
        log_e("realize audio recorder fail. %d.", res);
        return;
    }
    res = (*rec_obj)->GetInterface(rec_obj, SL_IID_RECORD, &rec_eng);
    if (res != SL_RESULT_SUCCESS) {
        (*eng_obj)->Destroy(eng_obj);
        eng_obj = nullptr;
        eng_eng = nullptr;
        (*rec_obj)->Destroy(rec_obj);
        rec_obj = nullptr;
        log_e("get audio recorder fail. %d.", res);
        return;
    }
    (*rec_eng)->SetPositionUpdatePeriod(rec_eng, PERIOD_TIME);
    (*rec_eng)->GetPositionUpdatePeriod(rec_eng, &period);
    log_d("get audio recorder period millisecond: %d", period);
    res = (*rec_obj)->GetInterface(rec_obj, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, &rec_queue);
    if (res != SL_RESULT_SUCCESS) {
        (*eng_obj)->Destroy(eng_obj);
        eng_obj = nullptr;
        eng_eng = nullptr;
        (*rec_obj)->Destroy(rec_obj);
        rec_obj = nullptr;
        log_e("get audio recorder queue fail. %d.", res);
        return;
    }
    res = (*rec_queue)->RegisterCallback(rec_queue, queue_callback, this);
    if (res != SL_RESULT_SUCCESS) {
        (*eng_obj)->Destroy(eng_obj);
        eng_obj = nullptr;
        eng_eng = nullptr;
        (*rec_obj)->Destroy(rec_obj);
        rec_obj = nullptr;
        log_e("audio recorder queue register callback fail. %d.", res);
        return;
    }
    log_d("init success.");
}

void media::audio_recorder::queue_callback(SLAndroidSimpleBufferQueueItf, void *ctx) {
    auto *rec = (audio_recorder *) ctx;
    rec->handle_frame();
    rec->enqueue(true);
}
