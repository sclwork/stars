package com.scliang.tars;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.AttributeSet;
import android.view.SurfaceHolder;

import androidx.annotation.Nullable;

public class MediaGLView extends GLSurfaceView {
    Runnable surfaceDestroyedRunnable;
    Runnable detachedFromWindowRunnable;

    public interface OnRendererListener {
        void onRendererInit();
        void onRendererRelease();
        void onRendererSurfaceCreated();
        void onRendererSurfaceChanged(int width, int height);
        void onRendererSurfaceDestroyed();
        void onRendererDrawFrame();
    }

    public MediaGLView(Context context) {
        super(context);
    }

    public MediaGLView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public final void surfaceDestroyed(SurfaceHolder holder) {
        super.surfaceDestroyed(holder);
        if (surfaceDestroyedRunnable != null) surfaceDestroyedRunnable.run();
        onSurfaceDestroyed(holder);
    }

    protected void onSurfaceDestroyed(SurfaceHolder holder) { }

    @Override
    protected final void onDetachedFromWindow() {
        if (detachedFromWindowRunnable != null) detachedFromWindowRunnable.run();
        super.onDetachedFromWindow();
        onDetachedFromWindow(null);
    }

    protected void onDetachedFromWindow(@Nullable Bundle args) { }
}
