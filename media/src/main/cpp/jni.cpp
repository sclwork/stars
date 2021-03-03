//
// Created by scliang on 1/5/21.
//

#include <jni.h>
#include "jni_bridge.h"

#define log_d(...)  LOG_D("Media-Native:jni", __VA_ARGS__)
#define log_e(...)  LOG_E("Media-Native:jni", __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_com_scliang_tars_Media_jniInit(
JNIEnv *env, jobject thiz,
jstring opencvCascadePath, jstring mnnModelPaths) {
//    log_d("RecorderJNI [Init]");
    const char *cascade = env->GetStringUTFChars(opencvCascadePath, nullptr);
    const char *mnn = env->GetStringUTFChars(mnnModelPaths, nullptr);
    // start main loop
    media::loop_start(cascade, mnn);
    env->ReleaseStringUTFChars(opencvCascadePath, cascade);
    env->ReleaseStringUTFChars(mnnModelPaths, mnn);
}

JNIEXPORT void JNICALL
Java_com_scliang_tars_Media_jniRelease(
JNIEnv *env, jobject thiz) {
    // exit main loop
    media::loop_exit();
//    log_d("RecorderJNI [Release]");
}

JNIEXPORT jint JNICALL
Java_com_scliang_tars_Media_jniRendererInit(
JNIEnv *env, jobject thiz) {
//    log_d("RecorderJNI [RendererInit]");
    media::renderer_init();
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_scliang_tars_Media_jniRendererRelease(
JNIEnv *env, jobject thiz) {
//    log_d("RecorderJNI [RendererRelease]");
    media::renderer_release();
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_scliang_tars_Media_jniRendererSurfaceCreated(
JNIEnv *env, jobject thiz) {
//    log_d("RecorderJNI [RendererSurfaceCreated]");
    media::renderer_surface_created();
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_scliang_tars_Media_jniRendererSurfaceChanged(
JNIEnv *env, jobject thiz,
jint width, jint height) {
//    log_d("RecorderJNI [RendererSurfaceChanged: %d,%d]", width, height);
    media::renderer_surface_changed(width, height);
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_scliang_tars_Media_jniRendererSurfaceDestroyed(
JNIEnv *env, jobject thiz) {
//    log_d("RecorderJNI [RendererSurfaceDestroyed]");
    media::renderer_surface_destroyed();
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_scliang_tars_Media_jniRendererDrawFrame(
JNIEnv *env, jobject thiz) {
//    log_d("RecorderJNI [RendererDrawFrame]");
    media::renderer_draw_frame();
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_scliang_tars_Media_jniStartVideoRecord(
JNIEnv *env, jobject thiz,
jstring mp4File) {
    const char *mp4_file = env->GetStringUTFChars(mp4File, nullptr);
    // start video record
    media::video_record_start(mp4_file);
    env->ReleaseStringUTFChars(mp4File, mp4_file);
    return 0;
}

JNIEXPORT jint JNICALL
Java_com_scliang_tars_Media_jniStopVideoRecord(
JNIEnv *env, jobject thiz) {
    // stop video record
    media::video_record_stop();
    return 0;
}

JNIEXPORT jboolean JNICALL
Java_com_scliang_tars_Media_jniVideoRecording(
JNIEnv *env, jobject thiz) {
    // check video recording
    return media::video_recording();
}

#ifdef __cplusplus
}
#endif
