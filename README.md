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

<br /><br />

---

- [opencv](https://github.com/opencv/opencv)
- [opensl-es](https://developer.android.google.cn/ndk/guides/audio/opensl)
- [ffmpeg](https://github.com/FFmpeg/FFmpeg)-4.3
- [webrtc](https://github.com/webrtc)-ns
- [tensorflow-lite](https://tensorflow.google.cn/lite/)
- [MNN](https://github.com/alibaba/MNN)
