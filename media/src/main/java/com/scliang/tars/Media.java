package com.scliang.tars;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.opengl.GLSurfaceView;

import androidx.annotation.NonNull;
import androidx.annotation.RawRes;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.SoftReference;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class Media {

    /**
     * Init media utils
     * @return true: load native library [media] success
     */
    public static boolean init(Context context) {
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

        // load so library
        try { System.loadLibrary("media"); r.mInit = true;
        } catch (Throwable e) { e.printStackTrace(); r.mInit = false; }

        // init ops
        if (r.mInit) {
            // jni init
            r.getGlslFiles();
            r.getLUTFileRes();
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

        // jni release
        r.jniRelease();
        // flag
        r.mInit = false;
    }

    /**
     * setup preview GLSurfaceView
     */
    public static void setMediaGLView(MediaGLView glView) {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        if (glView == null) {
            return;
        }

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
            }

            @Override
            public void onRendererSurfaceChanged(int width, int height) {
                Media.rendererSurfaceChanged(width, height);
            }

            @Override
            public void onRendererSurfaceDestroyed() {
                Media.rendererSurfaceDestroyed();
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

        // jni start video record
        r.jniStartVideoRecord(name);
    }

    /**
     * Stop Video Record
     */
    public static void stopVideoRecord() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        // jni stop video record
        r.jniStopVideoRecord();
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

        // jni check video record running
        return r.jniVideoRecording();
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
        getRawFile(R.raw.shader_frag_effect_heart,
                "shader_frag_effect_heart.glsl");
        getRawFile(R.raw.shader_frag_effect_lut,
                "shader_frag_effect_lut.glsl");
        getRawFile(R.raw.shader_frag_effect_ripple,
                "shader_frag_effect_ripple.glsl");
        getRawFile(R.raw.shader_frag_effect_flame,
                "shader_frag_effect_flame.glsl");
        getRawFile(R.raw.shader_frag_effect_burn,
                "shader_frag_effect_burn.glsl");
        getRawFile(R.raw.shader_frag_effect_distortedtv,
                "shader_frag_effect_distortedtv.glsl");
        getRawFile(R.raw.shader_frag_effect_distortedtv_box,
                "shader_frag_effect_distortedtv_box.glsl");
        getRawFile(R.raw.shader_frag_effect_distortedtv_glitch,
                "shader_frag_effect_distortedtv_glitch.glsl");
        getRawFile(R.raw.shader_frag_effect_distortedtv_crt,
                "shader_frag_effect_distortedtv_crt.glsl");
        getRawFile(R.raw.shader_frag_effect_eb,
                "shader_frag_effect_eb.glsl");
        getRawFile(R.raw.shader_frag_effect_sin_wave,
                "shader_frag_effect_sin_wave.glsl");
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
    ///////////////////////////////////////////////////////////
    private native int     jniStartVideoRecord(@NonNull String name);
    private native int     jniStopVideoRecord();
    private native boolean jniVideoRecording();




    ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////
    private SoftReference<Context> mContext;
    private SoftReference<MediaGLView> mGLView;
    private void setContext(Context context) { mContext = new SoftReference<>(context); }
    private Context getContext() { return mContext == null ? null : mContext.get(); }
    private static class SingletonHolder { private static final Media INSTANCE = new Media(); }
    private Media() { }
    private boolean mInit;
}
