package com.scliang.tars;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

public class DemoActivity extends AppCompatActivity {
//    private int mSelectedCamera = -1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Media.init(this);
        Window window = getWindow();
        window.getDecorView().setSystemUiVisibility(
//                View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR |
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
//                View.SYSTEM_UI_FLAG_FULLSCREEN |
                View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE);

        setContentView(R.layout.activity_main);
        final MediaGLView recorderView = findViewById(R.id.recorder_view);
        Media.setMediaGLView(recorderView);
//        if (recorderView != null) recorderView.setOnCameraCountListener(count -> {
//            RadioGroup group = findViewById(R.id.camera_ids);
//            group.setVisibility(View.INVISIBLE);
//            RadioButton rbA = group.findViewById(R.id.camera_a);
//            rbA.setVisibility(count > 0 ? View.VISIBLE : View.GONE);
//            RadioButton rbB = group.findViewById(R.id.camera_b);
//            rbB.setVisibility(count > 1 ? View.VISIBLE : View.GONE);
//            RadioButton rbC = group.findViewById(R.id.camera_c);
//            rbC.setVisibility(count > 2 ? View.VISIBLE : View.GONE);
//            RadioButton rbD = group.findViewById(R.id.camera_d);
//            rbD.setVisibility(count > 3 ? View.VISIBLE : View.GONE);
//            RadioButton rbE = group.findViewById(R.id.camera_e);
//            rbE.setVisibility(count > 4 ? View.VISIBLE : View.GONE);
//            RadioButton rbF = group.findViewById(R.id.camera_f);
//            rbF.setVisibility(count > 5 ? View.VISIBLE : View.GONE);
//            group.setOnCheckedChangeListener((group1, checkedId) -> {
//                if (checkedId == R.id.camera_a) {
//                    selectCamera(0);
//                } else if (checkedId == R.id.camera_b) {
//                    selectCamera(1);
//                } else if (checkedId == R.id.camera_c) {
//                    selectCamera(2);
//                } else if (checkedId == R.id.camera_d) {
//                    selectCamera(3);
//                } else if (checkedId == R.id.camera_e) {
//                    selectCamera(4);
//                } else if (checkedId == R.id.camera_f) {
//                    selectCamera(5);
//                }
//            });
//            group.setVisibility(View.VISIBLE);
//            if (mSelectedCamera < 0) {
//                if (count > 0) {
//                    rbA.setChecked(true);
//                }
//            } else {
//                selectCamera(mSelectedCamera);
//            }
//        });
    }

    @Override
    protected void onStart() {
        super.onStart();
        Media.onStart();
        acquireScreen();
    }

    @Override
    protected void onStop() {
        super.onStop();
        Media.onStop();
        releaseScreen();
    }

    @Override
    protected void onDestroy() {
        Media.release();
        super.onDestroy();
    }

//    private void selectCamera(int camera) {
//        mSelectedCamera = camera;
//    }

    private void acquireScreen() {
        final View view = findViewById(R.id.recorder_view);
        if (view != null) {
            view.post(() -> {
                Window window = getWindow();
                if (window != null) {
                    window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                }
            });
        }
    }

    private void releaseScreen() {
        final View view = findViewById(R.id.recorder_view);
        if (view != null) {
            view.post(() -> {
                Window window = getWindow();
                if (window != null) {
                    window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
                }
            });
        }
    }
}
