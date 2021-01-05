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

public class Recorder {
    /**
     * Get single instance object
     * @return single instance object
     */
    public static Recorder instance() { return Recorder.SingletonHolder.INSTANCE; }

    /**
     * Init media utils
     * @return true: load native library [media] success
     */
    public boolean init(Context context) {
        if (mInit)
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
        setContext(context);

        // load so library
        try { System.loadLibrary("recorder"); mInit = true;
        } catch (Throwable e) { e.printStackTrace(); mInit = false; }

        // init ops
        if (mInit) {
            // jni init
            jniInit(getOpenCVCascadeFileRes(), getMNNFileRes());
        }

        // return
        return mInit;
    }

    /**
     * Release [free/delete] all resource
     */
    public void release() {
        // must init [success] first
        if (!mInit)
            return;

        // jni release
        jniRelease();
        // flag
        mInit = false;
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
            File cascadeDir = context.getDir("cascade", Context.MODE_PRIVATE);
            File cascadeFile = new File(cascadeDir, "haarcascade_frontalface_default.xml");
            if (cascadeFile.exists())
                return cascadeFile.getAbsolutePath();

            os = new FileOutputStream(cascadeFile);
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = is.read(buffer)) != -1)
                os.write(buffer, 0, bytesRead);

            return cascadeFile.getAbsolutePath();
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
            File cascadeDir = context.getDir("cascade", Context.MODE_PRIVATE);
            File cascadeFile = new File(cascadeDir, name);
            if (cascadeFile.exists())
                return cascadeFile.getAbsolutePath();

            os = new FileOutputStream(cascadeFile);
            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = is.read(buffer)) != -1)
                os.write(buffer, 0, bytesRead);

            return cascadeFile.getAbsolutePath();
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




    ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////
    private native void jniInit(@NonNull String opencvCascadePath,
                                @NonNull String mnnModelPaths);
    private native void jniRelease();




    ///////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////
    private SoftReference<Context> mContext;
    private void setContext(Context context) { mContext = new SoftReference<>(context); }
    private Context getContext() { return mContext == null ? null : mContext.get(); }
    private static class SingletonHolder { private static final Recorder INSTANCE = new Recorder(); }
    private Recorder() { }
    private boolean mInit;
}
