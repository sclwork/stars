//
// Created by scliang on 1/5/21.
//

#include <jni.h>
#include "common.h"

#define d(...)  LOG_D("Recorder-Native:jni", __VA_ARGS__)
#define e(...)  LOG_E("Recorder-Native:jni", __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_com_scliang_tars_Recorder_jniInit(JNIEnv *env, jobject thiz,
jstring opencvCascadePath, jstring mnnModelPaths) {
//    d("RecorderJNI [Init]");
    const char *cascade = env->GetStringUTFChars(opencvCascadePath, nullptr);
    const char *mnn = env->GetStringUTFChars(mnnModelPaths, nullptr);
    // start main loop
    recorder_loop_start(cascade, mnn);
    env->ReleaseStringUTFChars(opencvCascadePath, cascade);
    env->ReleaseStringUTFChars(mnnModelPaths, mnn);
}

JNIEXPORT void JNICALL
Java_com_scliang_tars_Recorder_jniRelease(JNIEnv *env, jobject thiz) {
    // exit main loop
    recorder_loop_exit();
//    d("RecorderJNI [Release]");
}

#ifdef __cplusplus
}
#endif
