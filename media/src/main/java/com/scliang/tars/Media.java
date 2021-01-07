package com.scliang.tars;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;

import androidx.annotation.NonNull;
import androidx.annotation.RawRes;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.ref.SoftReference;

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
            r.getDemoCppFileRes(); // TODO: demo png res
            r.jniInit(r.getOpenCVCascadeFileRes(), r.getMNNFileRes());
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

    public static void rendererInit() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.jniRendererInit();
    }

    public static void rendererRelease() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.jniRendererRelease();
    }

    public static void rendererSurfaceCreated() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.jniRendererSurfaceCreated();
    }

    public static void rendererSurfaceChanged(int width, int height) {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.jniRendererSurfaceChanged(width, height);
    }

    public static void rendererSurfaceDestroyed() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.jniRendererSurfaceDestroyed();
    }

    public static void rendererDrawFrame() {
        final Media r = SingletonHolder.INSTANCE;
        // must init [success] first
        if (!r.mInit)
            return;

        r.jniRendererDrawFrame();
    }




    ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////
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

    private String getMNNFile(@RawRes int raw, String name) {
        InputStream is = null;
        FileOutputStream os = null;
        try {
            Context context = getContext();
            if (context == null)
                return "";

            is = context.getResources().openRawResource(raw);
            File dir = context.getDir("files", Context.MODE_PRIVATE);
            File file = new File(dir, name);
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

    private String getMNNFileRes() {
        return getMNNFile(R.raw.blazeface, "blazeface.mnn") + ";;;";
//        return ";" + getMNNFile(R.raw.det1, "det1.mnn") + ";" +
//                     getMNNFile(R.raw.det2, "det2.mnn") + ";" +
//                     getMNNFile(R.raw.det3, "det3.mnn");
    }

    private String getDemoCppFileRes() {
        InputStream is = null;
        FileOutputStream os = null;
        try {
            Context context = getContext();
            if (context == null)
                return "";

            is = context.getResources().openRawResource(R.raw.cpp);
            File dir = context.getDir("files", Context.MODE_PRIVATE);
            File file = new File(dir, "cpp.png");
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




    ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////
    private native void jniInit(@NonNull String opencvCascadePath,
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
    ///////////////////////////////////////////////////////////
    private SoftReference<Context> mContext;
    private void setContext(Context context) { mContext = new SoftReference<>(context); }
    private Context getContext() { return mContext == null ? null : mContext.get(); }
    private static class SingletonHolder { private static final Media INSTANCE = new Media(); }
    private Media() { }
    private boolean mInit;
}
