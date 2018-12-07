//
// Created by yijunwu on 2018/12/6.
//

#include <jni.h>
#include "x264_test.h"

FILE *out1;
int pts = 0;

JNIEXPORT void JNICALL
Java_com_ffmpeg_CameraActivity_init(JNIEnv *env, jobject instance, jint width, jint height, jint bitrate,
                                    jint orientation) {

    x264_init(width, height, bitrate, orientation);
    pts = 0;

    const char *outfile1 = "/sdcard/2222.h264";
    out1 = fopen(outfile1, "wb");
}

JNIEXPORT void JNICALL
Java_com_ffmpeg_CameraActivity_release(JNIEnv *env, jobject instance) {
    pts = 0;
    x264_release();

    fclose(out1);
}

JNIEXPORT void JNICALL
Java_com_ffmpeg_CameraActivity_encodeH264(JNIEnv *env, jobject instance, jbyteArray IV21, jint length) {
    jbyte *bytes = (*env)->GetByteArrayElements(env, IV21, NULL);

    char *I420 = (char *) (malloc(length * sizeof(char)));
    char *I420_90 = (char *) (malloc(length * sizeof(char)));

    NV21ToI420(I420, bytes, length);
    YUV420Rotate90(I420_90, I420, in_width, in_height);

    LOGE("NV21ToI420 length %d ", length)
    char *out = (char *) malloc(length * sizeof(char));
    int *len = malloc(10 * sizeof(int));
    int num_nals = encodeFrame(I420_90, pts, out, len);
    pts++;
    int frame_size = 0;
    for (int i = 0; i < num_nals; ++i) {
        frame_size += len[i];
    }
    fwrite(out, 1, frame_size, out1);
    LOGE("encodeFrame over 2");

    free(out);
    free(len);
    free(I420);
    free(I420_90);
    (*env)->ReleaseByteArrayElements(env, IV21, bytes, 0);
}

