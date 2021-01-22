## S-Tars

<br /><br />

#### Uses Permission

```
...
<uses-permission android:name="android.permission.CAMERA" />
<uses-permission android:name="android.permission.RECORD_AUDIO" />

<uses-feature android:glEsVersion="0x00030000" android:required="true" />
...
```

<br /><br />

#### Video Collect/Show

demo.xml
```
...
<com.scliang.tars.MediaGLView
        android:id="@+id/recorder_view"
        android:layout_width="match_parent"
        android:layout_height="match_parent" />
...
```

<br />

demo.java
```
@Override
protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    Media.init(this); // init media.
    ...
}

@Override
protected void onStart() {
    super.onStart();
    final MediaGLView recorderView = findViewById(R.id.recorder_view);
    if (recorderView != null) {
        recorderView.onResume(); // resume
    }
    ...
}

@Override
protected void onStop() {
    super.onStop();
    final MediaGLView recorderView = findViewById(R.id.recorder_view);
    if (recorderView != null) {
        recorderView.onPause(); // pause
    }
    ...
}

@Override
protected void onDestroy() {
    Media.release(); // release media.
    super.onDestroy();
    ...
}
```

<br /><br />

#### Select Camera

```
...
final MediaGLView recorderView = findViewById(R.id.recorder_view);
if (recorderView != null) {
    recorderView.setOnCameraCountListener(count -> {
        ...
        // count: camera count
        ...
    }
}
...
```

```
...
final MediaGLView recorderView = findViewById(R.id.recorder_view);
if (recorderView != null) {
    recorderView.selectCamera(camera); // select camera [0,count)
}
...
```

<br /><br />

#### Record Video

```
...
final MediaGLView recorderView = findViewById(R.id.recorder_view);
final TextView record = findViewById(R.id.record);
if (record != null) record.setOnClickListener(v -> {
    if (Media.rendererRecordRunning()) {
        if (recorderView != null) recorderView.stopRecord();
    } else {
        if (recorderView != null) recorderView.startRecord(getFilesDir() + "/demo.mp4");
    }
});
...
```

<br /><br /><br /><br />

<!--##### simple framework-->

<!--<span><div style="text-align: center;">-->
<!--![video-collect](image/video-collect.png)-->
<!--</div></span>-->

<!--<br /><br />-->

---

- [opencv](https://github.com/opencv/opencv)
- [opensl-es](https://developer.android.google.cn/ndk/guides/audio/opensl)
- [ffmpeg](https://github.com/FFmpeg/FFmpeg)-4.3
- [webrtc](https://github.com/webrtc)-ns
- [tensorflow-lite](https://tensorflow.google.cn/lite/)
- [MNN](https://github.com/alibaba/MNN)
