package com.scliang.tars;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.opengl.GLSurfaceView;
import android.text.TextUtils;

import androidx.annotation.NonNull;
import androidx.annotation.RawRes;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.SoftReference;
import java.util.ArrayList;
import java.util.List;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class Media {

    /**
     * Type
     */
    public enum Type {
        Record,
        Play,
    }

    /**
     * Camera
     */
    public enum Camera {
        Front(1),
        Back(0),

        ;Camera(int index) { mIndex = index; }
        private final int mIndex;
    }

    /**
     * Media State Listener
     */
    public interface OnMediaStateChangeListener {
        void onMediaSurfaceCreated();
        void onMediaSurfaceChanged(int width, int height);
        void onMediaSurfaceDestroyed();
    }

    /* SimpleStateChangeListener */
    public static abstract class SimpleStateChangeListener implements OnMediaStateChangeListener {
        @Override
        public void onMediaSurfaceCreated() {}
        @Override
        public void onMediaSurfaceChanged(int width, int height) {}
        @Override
        public void onMediaSurfaceDestroyed() {}
    }

    /**
     * Init media utils
     * @return true: load native library [media] success
     */
    public static boolean init(Context context) {
        return init(context, null);
    }

    /**
     * Init media utils
     * @return true: load native library [media] success
     */
    public static boolean init(Context context, OnMediaStateChangeListener listener) {
        final Media r = SingletonHolder.INSTANCE;
        if (r.mInit)
            return true;

        if (context == null)
            return false;

        // audio record permission
        if (PackageManager.PERMISSION_GRANTED !=
                context.checkSelfPermission(Manifest.permission.RECORD_AUDIO))
            return false;

        // camera permission
        if (PackageManager.PERMISSION_GRANTED !=
                context.checkSelfPermission(Manifest.permission.CAMERA))
            return false;

        // save context to SoftReference
        r.setContext(context);
        r.stateListener = new SoftReference<>(listener);

        // load so library
        try { System.loadLibrary("media"); r.mInit = true;
        } catch (Throwable e) { e.printStackTrace(); r.mInit = false; }

        // init ops
        if (r.mInit) {
            // clear
            effectNames.clear();
            // jni init
            r.getGlslFiles();
            r.getLUTFileRes();
            r.getRawFile(R.raw.ic_vid_file_not_exists, "ic_vid_file_not_exists.png");
            r.jniInit(r.getFileRootRes(), r.getOpenCVCascadeFileRes(), r.getMNNFileRes());
        }

        // return
        return r.mInit;
    }

    /**
     * Release [free/delete] all resource
     */
    public static void release() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.mGLView = null;
        r.stateListener = null;
        // jni release
        r.jniRelease();
        // flag
        r.mInit = false;
    }

    /**
     * get Supported Effect names
     */
    public static List<String> getSupportedEffectNames() {
        return effectNames;
    }

    /**
     * update/setup Supported Effect
     * @param name Supported Effect name
     */
    public static void updateEffectPaint(String name) {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        MediaGLView glView = r.mGLView == null ? null : r.mGLView.get();
        if (glView == null)
            return;

        // jni update effect paint
        glView.queueEvent(()->r.jniRendererUpdatePaint(name));
    }

    /**
     * setup preview GLSurfaceView
     */
    public static void setMediaGLView(MediaGLView glView) {
        setMediaGLView(glView, Type.Play, Camera.Back);
    }

    /**
     * setup preview GLSurfaceView
     */
    public static void setMediaGLView(MediaGLView glView, Type type) {
        setMediaGLView(glView, type, Camera.Back);
    }

    /**
     * setup preview GLSurfaceView
     */
    public static void setMediaGLView(MediaGLView glView, Camera defCamera) {
        setMediaGLView(glView, Type.Record, defCamera);
    }

    /**
     * setup preview GLSurfaceView
     */
    public static void setMediaGLView(MediaGLView glView, Type type, Camera defCamera) {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        if (glView == null) {
            return;
        }

        // setup default type
        Media.selType = type;
        // setup default camera
        Media.selCamera = defCamera;

        RecorderRenderer renderer = new RecorderRenderer(new MediaGLView.OnRendererListener() {
            @Override
            public void onRendererInit() {
                Media.rendererInit();
            }

            @Override
            public void onRendererRelease() {
                Media.rendererRelease();
            }

            @Override
            public void onRendererSurfaceCreated() {
                Media.rendererSurfaceCreated();
                // callback
                OnMediaStateChangeListener lis =
                        SingletonHolder.INSTANCE.stateListener==null?null:
                                SingletonHolder.INSTANCE.stateListener.get();
                if (lis != null) lis.onMediaSurfaceCreated();
            }

            @Override
            public void onRendererSurfaceChanged(int width, int height) {
                Media.rendererSurfaceChanged(width, height);
                // only record type
                if (selType == Type.Record) {
                    Media.switchCamera(Media.selCamera);
                }
                // callback
                OnMediaStateChangeListener lis =
                        SingletonHolder.INSTANCE.stateListener==null?null:
                                SingletonHolder.INSTANCE.stateListener.get();
                if (lis != null) lis.onMediaSurfaceChanged(width, height);
            }

            @Override
            public void onRendererSurfaceDestroyed() {
                Media.rendererSurfaceDestroyed();
                // callback
                OnMediaStateChangeListener lis =
                        SingletonHolder.INSTANCE.stateListener==null?null:
                                SingletonHolder.INSTANCE.stateListener.get();
                if (lis != null) lis.onMediaSurfaceDestroyed();
            }

            @Override
            public void onRendererDrawFrame() {
                Media.rendererDrawFrame();
            }
        });

        glView.detachedFromWindowRunnable = ()-> glView.queueEvent(renderer::release);
        glView.surfaceDestroyedRunnable = ()-> glView.queueEvent(renderer::onSurfaceDestroyed);
        glView.setEGLContextClientVersion(3);
        glView.setRenderer(renderer);
        glView.queueEvent(renderer::init);
        glView.setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        r.mGLView = new SoftReference<>(glView);
    }

    // call in jni - can't proguard
    private static void requestRender(int code) {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        MediaGLView glView = r.mGLView == null ? null : r.mGLView.get();
        if (glView != null) glView.requestRender();
    }

    public static void onStart() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        MediaGLView glView = r.mGLView == null ? null : r.mGLView.get();
        if (glView != null) glView.onResume();
    }

    public static void onStop() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        MediaGLView glView = r.mGLView == null ? null : r.mGLView.get();
        if (glView != null) glView.onPause();
    }

    /**
     * Start Video Record
     * @param name [.mp4]  save video to mp4 file   (/sdcard/demo.mp4)
     *             [rtmp:] rtmp video stream to url (rtmp://127.0.0.1/live/demo)
     */
    public static void startVideoRecord(@NonNull String name) {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        // only record type
        if (selType != Type.Record) {
            return;
        }

        MediaGLView glView = r.mGLView == null ? null : r.mGLView.get();
        if (glView == null)
            return;

        // jni start video record
        glView.queueEvent(()->r.jniStartVideoRecord(name));
    }

    /**
     * Stop Video Record
     */
    public static void stopVideoRecord() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        // only record type
        if (selType != Type.Record) {
            return;
        }

        MediaGLView glView = r.mGLView == null ? null : r.mGLView.get();
        if (glView == null)
            return;

        // jni stop video record
        glView.queueEvent(r::jniStopVideoRecord);
    }

    /**
     * Check VideoRecord Running
     * @return true: recording
     */
    public static boolean isVideoRecording() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return false;

        // only record type
        if (selType != Type.Record) {
            return false;
        }

        // jni check video record running
        return r.jniVideoRecording();
    }

    /**
     * switch front/back camera
     */
    public static void switchCamera() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        // only record type
        if (selType != Type.Record) {
            return;
        }

        if (selCamera == Camera.Back) {
            switchCamera(Camera.Front);
        } else if (selCamera == Camera.Front) {
            switchCamera(Camera.Back);
        }
    }

    /**
     * switch front/back camera
     */
    public static void switchCamera(Camera camera) {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        // only record type
        if (selType != Type.Record) {
            return;
        }

        MediaGLView glView = r.mGLView == null ? null : r.mGLView.get();
        if (glView == null)
            return;

        // jni switch front/back camera
        glView.queueEvent(()->{
            int res = r.jniCameraSelect(camera.mIndex);
            if (res == 0) { selCamera = camera; }
        });
    }

    /**
     * Start Video Play
     * @param name [.mp4]  play video from mp4 file   (/sdcard/demo.mp4)
     *             [rtmp:] rtmp video stream from url (rtmp://127.0.0.1/live/demo)
     */
    public static void startVideoPlay(@NonNull String name) {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        // only record type
        if (selType != Type.Play) {
            return;
        }

        MediaGLView glView = r.mGLView == null ? null : r.mGLView.get();
        if (glView == null)
            return;

        // jni start video play
        glView.queueEvent(()->r.jniStartVideoPlay(name));
    }

    /**
     * Stop Video Play
     */
    public static void stopVideoPlay() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        // only record type
        if (selType != Type.Play) {
            return;
        }

        MediaGLView glView = r.mGLView == null ? null : r.mGLView.get();
        if (glView == null)
            return;

        // jni stop video play
        glView.queueEvent(r::jniStopVideoPlay);
    }

    /**
     * Check VideoPlay Running
     * @return true: playing
     */
    public static boolean isVideoPlaying() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return false;

        // only record type
        if (selType != Type.Play) {
            return false;
        }

        // jni check video play running
        return r.jniVideoPlaying();
    }

    private static class RecorderRenderer implements GLSurfaceView.Renderer {
        private final MediaGLView.OnRendererListener mOnRendererListener;

        public RecorderRenderer(MediaGLView.OnRendererListener listener) {
            mOnRendererListener = listener;
        }

        public final void init() {
            if (mOnRendererListener != null)
                mOnRendererListener.onRendererInit();
        }

        public final void release() {
            if (mOnRendererListener != null)
                mOnRendererListener.onRendererRelease();
        }

        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            if (mOnRendererListener != null)
                mOnRendererListener.onRendererSurfaceCreated();
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            if (mOnRendererListener != null)
                mOnRendererListener.onRendererSurfaceChanged(width, height);
        }

        public void onSurfaceDestroyed() {
            if (mOnRendererListener != null)
                mOnRendererListener.onRendererSurfaceDestroyed();
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            if (mOnRendererListener != null)
                mOnRendererListener.onRendererDrawFrame();
        }
    }

    private static void rendererInit() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.jniRendererInit();
    }

    private static void rendererRelease() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.jniRendererRelease();
    }

    private static void rendererSurfaceCreated() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.jniRendererSurfaceCreated();
    }

    private static void rendererSurfaceChanged(int width, int height) {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.jniRendererSurfaceChanged(width, height);
    }

    private static void rendererSurfaceDestroyed() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.jniRendererSurfaceDestroyed();
    }

    private static void rendererDrawFrame() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.jniRendererDrawFrame();
    }




    ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////
    private String getFileRootRes() {
        try {
            Context context = getContext();
            if (context == null)
                return "";
            File dir = context.getDir("files", Context.MODE_PRIVATE);
            return dir.getAbsolutePath();
        } catch (Exception e) {
            e.printStackTrace();
            return "";
        }
    }

    private String getOpenCVCascadeFileRes() {
        InputStream is = null;
        FileOutputStream os = null;
        try {
            Context context = getContext();
            if (context == null)
                return "";

            is = context.getResources().openRawResource(R.raw.haarcascade_frontalface_default);
            File dir = context.getDir("files", Context.MODE_PRIVATE);
            File file = new File(dir, "haarcascade_frontalface_default.xml");
            if (file.exists())
                return file.getAbsolutePath();

            os = new FileOutputStream(file);
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = is.read(buffer)) != -1)
                os.write(buffer, 0, bytesRead);

            return file.getAbsolutePath();
        } catch (IOException e) {
            e.printStackTrace();
            return "";
        } finally {
            try { if (is != null) is.close();
            } catch (IOException ignored) { }
            try { if (os != null) os.close();
            } catch (IOException ignored) { }
        }
    }

    private String getRawFile(@RawRes int raw, String name) {
        InputStream is = null;
        FileOutputStream os = null;
        try {
            Context context = getContext();
            if (context == null)
                return "";

            if (TextUtils.isEmpty(name)) {
                return "";
            }

            if (name.contains("shader_frag_effect_")) {
                effectNames.add(
                        name.replace("shader_frag_effect_", "")
                            .replace(".glsl", "")
                            .toUpperCase());
            }

            is = context.getResources().openRawResource(raw);
            File dir = context.getDir("files", Context.MODE_PRIVATE);
            File file = new File(dir, name);
            if (file.exists()) file.delete();
            os = new FileOutputStream(file);
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = is.read(buffer)) != -1)
                os.write(buffer, 0, bytesRead);

            return file.getAbsolutePath();
        } catch (IOException e) {
            e.printStackTrace();
            return "";
        } finally {
            try { if (is != null) is.close();
            } catch (IOException ignored) { }
            try { if (os != null) os.close();
            } catch (IOException ignored) { }
        }
    }

    private String getMNNFileRes() {
        return getRawFile(R.raw.blazeface, "blazeface.mnn") + ";;;";
//        return getRawFile(R.raw.det1, "det1.mnn") + ";" +
//               getRawFile(R.raw.det2, "det2.mnn") + ";" +
//               getRawFile(R.raw.det3, "det3.mnn");
    }

    private void getLUTFileRes() {
        getRawFile(R.raw.lut_a, "lut_a.png");
        getRawFile(R.raw.lut_b, "lut_b.png");
        getRawFile(R.raw.lut_c, "lut_c.png");
    }

    private void getGlslFiles() {
        getRawFile(R.raw.shader_frag_none,
                "shader_frag_none.glsl");
        getRawFile(R.raw.shader_frag_effect_none,
                "shader_frag_effect_none.glsl");
        getRawFile(R.raw.shader_frag_effect_face,
                "shader_frag_effect_face.glsl");
//        getRawFile(R.raw.shader_frag_effect_heart,
//                "shader_frag_effect_heart.glsl");
//        getRawFile(R.raw.shader_frag_effect_lut,
//                "shader_frag_effect_lut.glsl");
        getRawFile(R.raw.shader_frag_effect_ripple,
                "shader_frag_effect_ripple.glsl");
//        getRawFile(R.raw.shader_frag_effect_flame,
//                "shader_frag_effect_flame.glsl");
//        getRawFile(R.raw.shader_frag_effect_burn,
//                "shader_frag_effect_burn.glsl");
        getRawFile(R.raw.shader_frag_effect_distortedtv,
                "shader_frag_effect_distortedtv.glsl");
        getRawFile(R.raw.shader_frag_effect_distortedtv_box,
                "shader_frag_effect_distortedtv_box.glsl");
        getRawFile(R.raw.shader_frag_effect_distortedtv_glitch,
                "shader_frag_effect_distortedtv_glitch.glsl");
        getRawFile(R.raw.shader_frag_effect_distortedtv_crt,
                "shader_frag_effect_distortedtv_crt.glsl");
//        getRawFile(R.raw.shader_frag_effect_eb,
//                "shader_frag_effect_eb.glsl");
//        getRawFile(R.raw.shader_frag_effect_sin_wave,
//                "shader_frag_effect_sin_wave.glsl");
        getRawFile(R.raw.shader_frag_effect_floyd,
                "shader_frag_effect_floyd.glsl");
        getRawFile(R.raw.shader_frag_effect_3basic,
                "shader_frag_effect_3basic.glsl");
        getRawFile(R.raw.shader_frag_effect_3floyd,
                "shader_frag_effect_3floyd.glsl");
        getRawFile(R.raw.shader_frag_effect_pagecurl,
                "shader_frag_effect_pagecurl.glsl");
        getRawFile(R.raw.shader_frag_effect_old_video,
                "shader_frag_effect_old_video.glsl");
        getRawFile(R.raw.shader_frag_effect_crosshatch,
                "shader_frag_effect_crosshatch.glsl");
        getRawFile(R.raw.shader_frag_effect_cmyk,
                "shader_frag_effect_cmyk.glsl");
        getRawFile(R.raw.shader_frag_effect_drawing,
                "shader_frag_effect_drawing.glsl");
        getRawFile(R.raw.shader_frag_effect_neon,
                "shader_frag_effect_neon.glsl");
        getRawFile(R.raw.shader_frag_effect_fisheye,
                "shader_frag_effect_fisheye.glsl");
        getRawFile(R.raw.shader_frag_effect_barrelblur,
                "shader_frag_effect_barrelblur.glsl");
        getRawFile(R.raw.shader_frag_effect_fastblur,
                "shader_frag_effect_fastblur.glsl");
        getRawFile(R.raw.shader_frag_effect_illustration,
                "shader_frag_effect_illustration.glsl");
        getRawFile(R.raw.shader_frag_effect_hexagon,
                "shader_frag_effect_hexagon.glsl");
        getRawFile(R.raw.shader_frag_effect_sobel,
                "shader_frag_effect_sobel.glsl");
        getRawFile(R.raw.shader_frag_effect_lens,
                "shader_frag_effect_lens.glsl");
        getRawFile(R.raw.shader_frag_effect_float_camera,
                "shader_frag_effect_float_camera.glsl");
        getRawFile(R.raw.shader_vert_none,
                "shader_vert_none.glsl");
        getRawFile(R.raw.shader_vert_effect_none,
                "shader_vert_effect_none.glsl");
    }




    ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////
    private native void jniInit(@NonNull String fileRoot,
                                @NonNull String opencvCascadePath,
                                @NonNull String mnnModelPaths);
    private native void jniRelease();
    ///////////////////////////////////////////////////////////
    private native int jniRendererInit();
    private native int jniRendererRelease();
    private native int jniRendererSurfaceCreated();
    private native int jniRendererSurfaceChanged(int width, int height);
    private native int jniRendererSurfaceDestroyed();
    private native int jniRendererDrawFrame();
    private native int jniRendererUpdatePaint(@NonNull String name);
    ///////////////////////////////////////////////////////////
    private native int     jniStartVideoRecord(@NonNull String name);
    private native int     jniStopVideoRecord();
    private native boolean jniVideoRecording();
    ///////////////////////////////////////////////////////////
    private native int jniCameraSelect(int camera);
    ///////////////////////////////////////////////////////////
    private native int     jniStartVideoPlay(@NonNull String name);
    private native int     jniStopVideoPlay();
    private native boolean jniVideoPlaying();




    ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////
    private SoftReference<Context> mContext;
    private SoftReference<MediaGLView> mGLView;
    private SoftReference<OnMediaStateChangeListener> stateListener;
    private void setContext(Context context) { mContext = new SoftReference<>(context); }
    private Context getContext() { return mContext == null ? null : mContext.get(); }
    private static class SingletonHolder { private static final Media INSTANCE = new Media(); }
    private static Camera selCamera = Camera.Back; // default back camera
    private static Type selType = Type.Record; // default record
    private final static List<String> effectNames = new ArrayList<>();
    private Media() { }
    private boolean mInit;
}
