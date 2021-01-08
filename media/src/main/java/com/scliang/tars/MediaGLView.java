package com.scliang.tars;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.SurfaceHolder;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MediaGLView extends GLSurfaceView {
    public interface OnRendererListener {
        void onRendererInit();
        void onRendererRelease();
        void onRendererSurfaceCreated();
        void onRendererSurfaceChanged(int width, int height);
        void onRendererSurfaceDestroyed();
        void onRendererDrawFrame();
    }

    private static class RecorderRenderer implements GLSurfaceView.Renderer {
        private final OnRendererListener mOnRendererListener;

        public RecorderRenderer(OnRendererListener listener) {
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

    private RecorderRenderer mRenderer;

    public MediaGLView(Context context) {
        super(context);
        init();
    }

    public MediaGLView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    private void init() {
        setEGLContextClientVersion(3);
        mRenderer = new RecorderRenderer(new OnRendererListener() {
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
        setRenderer(mRenderer);
        queueEvent(mRenderer::init);
        setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        super.surfaceDestroyed(holder);
        queueEvent(()->{if(mRenderer != null)mRenderer.onSurfaceDestroyed();});
    }

    @Override
    protected void onDetachedFromWindow() {
        queueEvent(()->{if(mRenderer != null)mRenderer.release();});
        super.onDetachedFromWindow();
    }
}
