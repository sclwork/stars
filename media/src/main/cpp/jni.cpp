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

static JavaVM *g_JavaVM = nullptr;
static jobject  g_MediaClass = nullptr;

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_JavaVM = vm;
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved) {
    g_JavaVM = nullptr;
}

JNIEXPORT void JNICALL
Java_com_scliang_tars_Media_jniInit(
JNIEnv *env, jobject thiz,
jstring fileRootPath, jstring opencvCascadePath, jstring mnnModelPaths) {
//    log_d("RecorderJNI [Init]");
    jclass mediaClass = env->FindClass("com/scliang/tars/Media");
    if (mediaClass != nullptr) {
        g_MediaClass = env->NewGlobalRef(mediaClass);
    }
    const char *file = env->GetStringUTFChars(fileRootPath, nullptr);
    const char *cascade = env->GetStringUTFChars(opencvCascadePath, nullptr);
    const char *mnn = env->GetStringUTFChars(mnnModelPaths, nullptr);
    // start main loop
    media::loop_start(file, cascade, mnn, [](int32_t code) {
        JNIEnv *p_env = nullptr;
        if (g_JavaVM != nullptr) {
            g_JavaVM->AttachCurrentThread(&p_env, nullptr);
        }
        if (p_env == nullptr || g_MediaClass == nullptr) {
            return;
        }
        auto mediaClass = (jclass) g_MediaClass;
        if (mediaClass == nullptr) {
            return;
        }
        jmethodID mediaRRID = p_env->GetStaticMethodID(mediaClass, "requestRender", "(I)V");
        if (mediaRRID != nullptr) {
            p_env->CallStaticVoidMethod(mediaClass, mediaRRID, code);
        }
        if (g_JavaVM != nullptr) {
            g_JavaVM->DetachCurrentThread();
        }
    });
    env->ReleaseStringUTFChars(fileRootPath, file);
    env->ReleaseStringUTFChars(opencvCascadePath, cascade);
    env->ReleaseStringUTFChars(mnnModelPaths, mnn);
}

JNIEXPORT void JNICALL
Java_com_scliang_tars_Media_jniRelease(
JNIEnv *env, jobject thiz) {
    if (g_MediaClass != nullptr) {
        env->DeleteGlobalRef(g_MediaClass);
    }
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
Java_com_scliang_tars_Media_jniRendererUpdatePaint(
JNIEnv *env, jobject thiz,
jstring name) {
    const char *n = env->GetStringUTFChars(name, nullptr);
    // update effect paint
    media::renderer_updt_paint(n);
    env->ReleaseStringUTFChars(name, n);
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

JNIEXPORT jint JNICALL
Java_com_scliang_tars_Media_jniCameraSelect(
JNIEnv *env, jobject thiz,
jint camera) {
    // select camera
    media::camera_select(camera);
    return 0;
}

#ifdef __cplusplus
}
#endif
