package com.scliang.tars;

import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;

import java.util.List;

public class PlayerActivity extends AppCompatActivity {

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
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

        setContentView(R.layout.activity_player);
        final MediaGLView recorderView = findViewById(R.id.player_view);
        Media.setMediaGLView(recorderView, Media.Type.Play);
        final Button effect = findViewById(R.id.effect);
        if (effect != null) effect.setOnClickListener(v -> {
            List<String> names = Media.getSupportedEffectNames();
            String[] items = new String[names.size()];
            for (int i = 0; i < names.size(); i++) {
                String name = names.get(i);
                items[i] = name;
//                Log.d("Media-Native", "EffectName: " + name);
            }
            AlertDialog.Builder listDialog = new AlertDialog.Builder(this);
            listDialog.setItems(items, (dialog, which) -> {
                String name = items[which];
                effect.setText(name);
                Media.updateEffectPaint(name);
            });
            listDialog.show();
        });
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

    private void acquireScreen() {
        final View view = findViewById(R.id.player_view);
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
        final View view = findViewById(R.id.player_view);
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
