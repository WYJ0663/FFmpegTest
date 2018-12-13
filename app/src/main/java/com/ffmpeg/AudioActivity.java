package com.ffmpeg;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import com.example.ffmpeg.R;

public class AudioActivity extends Activity {
    private Button mRecordVie;

    private boolean mIsRecord = false;

    private AudioManager mAudioManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_audio);

        mRecordVie = (Button) findViewById(R.id.record);
        mRecordVie.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mIsRecord = !mIsRecord;
                if (mIsRecord) {
                    mRecordVie.setText("stop");
                    mAudioManager = new AudioManager();
                    mAudioManager.start();
                } else {
                    mRecordVie.setText("start");
                    if (mAudioManager != null) {
                        mAudioManager.stop();
                    }
                }
            }
        });


    }

}
