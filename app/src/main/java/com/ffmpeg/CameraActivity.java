package com.ffmpeg;

import android.app.Activity;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import com.example.ffmpeg.R;

import java.io.IOException;
import java.util.List;

public class CameraActivity extends Activity implements SurfaceHolder.Callback {

    private SurfaceView surfaceView;
    private Camera camera;
    private Button mRecordVie;
    private boolean mIsRecord = false;

    private Camera.Parameters parameters;

    int width = 1920;

    int height = 1080;

    int framerate = 30;

    int biterate = 8500 * 1000;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);

        surfaceView = (SurfaceView) findViewById(R.id.surface_view);
        surfaceView.getHolder().addCallback(this);

        // 打开摄像头并将展示方向旋转90度
        camera = Camera.open();


        mRecordVie = (Button) findViewById(R.id.record);
        mIsRecord = false;
        mRecordVie.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mIsRecord = !mIsRecord;
                if (mIsRecord) {
                    mRecordVie.setText("stop");
                } else {
                    mRecordVie.setText("start");
                }
            }
        });
    }

    //------ Surface 预览 -------
    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        init(width, height, biterate, 90);
        try {
            camera.setDisplayOrientation(90);
            camera.cancelAutoFocus();
            // setCameraDisplayOrientation(this, 0, camera);
            parameters = camera.getParameters();
            parameters.setPreviewFormat(ImageFormat.NV21);
            parameters.setPreviewSize(width, height);
            parameters.setPictureSize(width, height);
//            for (String s : parameters.getSupportedFocusModes()) {
//                if (Camera.Parameters.FOCUS_MODE_AUTO.equals(s)) {
//                    parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
//                }
//            }
            parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
            camera.setParameters(parameters);
            camera.setPreviewDisplay(surfaceHolder);
            camera.startPreview();
            camera.setPreviewCallback(new Camera.PreviewCallback() {
                @Override
                public void onPreviewFrame(byte[] bytes, Camera camera) {
                    // 获取NV21数据
                    if (mIsRecord) {
                        encodeH264(bytes, bytes.length);
                    }
                }
            });
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    private static Camera.Size getOptimalSize( List<Camera.Size> sizes, int w, int h) {
        final double ASPECT_TOLERANCE = 0.1;
        double targetRatio = (double) h / w;
        Camera.Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;

        int targetHeight = h;

        for (Camera.Size size : sizes) {
            double ratio = (double) size.width / size.height;
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) continue;
            if (Math.abs(size.height - targetHeight) < minDiff) {
                optimalSize = size;
                minDiff = Math.abs(size.height - targetHeight);
            }
        }

        if (optimalSize == null) {
            minDiff = Double.MAX_VALUE;
            for (Camera.Size size : sizes) {
                if (Math.abs(size.height - targetHeight) < minDiff) {
                    optimalSize = size;
                    minDiff = Math.abs(size.height - targetHeight);
                }
            }
        }

        return optimalSize;
    }


    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int format, int w, int h) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
        camera.release();
    }

    static {
        System.loadLibrary("x2641");
        System.loadLibrary("native-lib");
    }

    public native void encodeH264(byte[] bytes, int length);

    //orientation 0, 90, 180, and 270.
    public native void init(int width, int height, int bitrate, int orientation);

    public native void release();
}
