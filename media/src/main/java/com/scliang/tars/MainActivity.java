package com.scliang.tars;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        final View recorder = findViewById(R.id.recorder);
        if (recorder != null) recorder.setOnClickListener(v->
                startActivity(new Intent(this, RecorderActivity.class)));
        final View player = findViewById(R.id.player);
        if (player != null) player.setOnClickListener(v->
                startActivity(new Intent(this, PlayerActivity.class)));
    }
}
