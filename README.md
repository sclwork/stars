## S-Tars

<br /><br />

| 功能 |  |  |  |  |  |  |
| --- | --- | --- | --- | --- | --- | --- |
| 图像采集 | camera2ndk | libyuv | mnn |  |  |  |
| 声音采集 | OpenSLES |  |  |  |  |
| 图像特效 |  |  |  |  |  | glsl |
| 图像预览 |  |  | OpenGLES | GLSurfaceView | FBO | Kalman |
| 图像编码 | x264 | ffmpeg |  |  |  |
| 声音编码 | pcm | loudnorm | webrtc-ns | aac |  |  |
| 导出Mp4 |  |  |  |  |  | ffmpeg |
| RTMP推流 |  |  |  |  |  | ffmpeg |

<br /><br />

#### Uses Permission

```
...
<uses-permission android:name="android.permission.INTERNET" />
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
    MediaGLView recorderView = findViewById(R.id.recorder_view);
    Media.setMediaGLView(recorderView); // setup MediaGLView
    ...
}

@Override
protected void onStart() {
    super.onStart();
    Media.onStart(); // start/resume media.
    ...
}

@Override
protected void onStop() {
    super.onStop();
    Media.onStop(); // stop/pause media.
    ...
}

@Override
protected void onDestroy() {
    Media.release(); // release media.
    super.onDestroy();
    ...
}
```

<br />

```
// record to mp4
View record = findViewById(R.id.record);
record.setOnClickListener(v -> {
    if (Media.isVideoRecording()) {
        Media.stopVideoRecord();
    } else {
        Media.startVideoRecord(getFilesDir() + "/demo.mp4");
    }
});
```

<br />

```
// record to rtmp
View record = findViewById(R.id.record);
record.setOnClickListener(v -> {
    if (Media.isVideoRecording()) {
        Media.stopVideoRecord();
    } else {
        Media.startVideoRecord("rtmp://scliang.com/live/demo");
    }
});
```

<br /><br />

---

- [opencv](https://github.com/opencv/opencv)
- [opensl-es](https://developer.android.google.cn/ndk/guides/audio/opensl)
- [ffmpeg](https://github.com/FFmpeg/FFmpeg)-4.3
- [webrtc](https://github.com/webrtc)-ns
- [tensorflow-lite](https://tensorflow.google.cn/lite/)
- [MNN](https://github.com/alibaba/MNN)
