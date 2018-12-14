package com.ffmpeg;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import org.jetbrains.annotations.NotNull;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * fdk-aac软编码
 */
public class AudioManager {
    private AudioRecord mAudioRecord = null;
    private boolean mIsRecording;

    public static final int SAMPLE_RATE = 44100;
    public static final int CHANNELS = 2;
    public static final int BIT_RATE = 64000;

    int audioSource = MediaRecorder.AudioSource.MIC;
    int channelConfig = AudioFormat.CHANNEL_IN_STEREO;
    int audioEncoding = AudioFormat.ENCODING_PCM_16BIT;
    int bufferSizeInBytes = AudioRecord.getMinBufferSize(SAMPLE_RATE,
            channelConfig, audioEncoding);

    HandlerThread mHandlerThread ;
    Handler mHandler;
    public AudioManager() {
        mHandlerThread = new HandlerThread("audio");
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper());
        create();
    }

    public void create() {
        bufferSizeInBytes = AudioRecord.getMinBufferSize(SAMPLE_RATE, channelConfig, audioEncoding);
        mAudioRecord = new AudioRecord(audioSource, SAMPLE_RATE, channelConfig, audioEncoding, bufferSizeInBytes);
        mBufferSize = init(CHANNELS, SAMPLE_RATE, BIT_RATE);
    }

    public void start() {
        if (mAudioRecord != null) {

            mAudioRecord.startRecording();
            mIsRecording = true;

            new Thread(new Runnable() {
                @Override
                public void run() {
                    read();
                }
            }).start();
        }
    }

    private int mBufferSize;

    public void read() {
        openStream();
        byte audioData[] = new byte[mBufferSize];
        while (mIsRecording) {
            //获取PCM数据
            int read = mAudioRecord.read(audioData, 0, mBufferSize);
            // 如果读取音频数据没有出现错误
            if (AudioRecord.ERROR_INVALID_OPERATION != read) {
//                final byte[] ralAudio = new byte[read];
                //每次录音读取4K数据
//                System.arraycopy(audioData, 0, ralAudio, 0, read);

                encodeAAC(audioData, audioData.length);
                writeStream(audioData);
                Log.e("yijun", "read " + read);
            }
        }
        closeStream();
    }

    private void over() {
        if (mAudioRecord != null) {
            if (null != mAudioRecord) {
                mAudioRecord.stop();
                mAudioRecord.release();
            }
        }
        release();
    }

    public void stop() {
        mIsRecording = false;
        over();
    }

    static {
        System.loadLibrary("fdk-aac");
        System.loadLibrary("native-lib");
    }

    public native int init(int channels, int sampleRate, int bitRate);

    public native void encodeAAC(byte[] bytes, int length);

    public native void release();


    ////////////////////////////////////////////////////////////////////////////////////
    String fileName = Environment.getExternalStorageDirectory().getPath() + "/22test_audio.pcm";
    String fileName2 = Environment.getExternalStorageDirectory().getPath() + "/22test_audio.wav";

    FileOutputStream mFileOutputStream;

    boolean isDebug = true;

    void openStream() {
        if (!isDebug) {
            return;
        }
        initFile(fileName);
        initFile(fileName2);
        Log.e("yijun", "fileName " + fileName );
        Log.e("yijun", "fileName2 " + fileName2);
        try {
            mFileOutputStream = new FileOutputStream(fileName);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
            Log.e("yijun", "mFileOutputStream " + fileName);
        }
    }

    @NotNull
    private File initFile(String fileName) {
        File file = new File(fileName);
        if (file.exists()) {
            file.delete();
        }
        try {
            file.createNewFile();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return file;
    }

    void writeStream(byte[] data) {
        if (!isDebug) {
            return;
        }
        try {
            mFileOutputStream.write(data);
        } catch (IOException e) {
            e.printStackTrace();
            Log.e("yijun", "writeStream " + e.toString());
        }
    }

    void closeStream() {
        if (!isDebug) {
            return;
        }
        try {
            mFileOutputStream.close();
        } catch (IOException e) {
            e.printStackTrace();
        }

        new PcmToWavUtil(SAMPLE_RATE, channelConfig, audioEncoding).pcmToWav(fileName, fileName2);
    }
}
