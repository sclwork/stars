package com.scliang.tars;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.view.Window;

public class DemoActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Media.init(this);
        Window window = getWindow();
        window.getDecorView().setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR |
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
                View.SYSTEM_UI_FLAG_FULLSCREEN |
                View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE);

        setContentView(R.layout.activity_main);
    }

    @Override
    protected void onStart() {
        super.onStart();
        final MediaGLView recorderView = findViewById(R.id.recorder_view);
        if (recorderView != null) {
            recorderView.onResume();
        }
    }

    @Override
    protected void onStop() {
        super.onStop();
        final MediaGLView recorderView = findViewById(R.id.recorder_view);
        if (recorderView != null) {
            recorderView.onPause();
        }
    }

    @Override
    protected void onDestroy() {
        Media.release();
        super.onDestroy();
    }
}
