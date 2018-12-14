//
// Created by yijunwu on 2018/12/10.
//


#include <jni.h>
#include <malloc.h>
#include <memory.h>
#include "fdk_aac_test.h"
#include "acc.h"

const char *outfile1 = "/sdcard/2222.aac";

FILE *out1;

JNIEXPORT jint JNICALL
Java_com_ffmpeg_AudioManager_init(JNIEnv *env, jobject instance, jint channels, jint sampleRate, jint bitRate) {

    aac_init_param(channels, sampleRate, bitRate);
    int size = aac_init();

    out1 = fopen(outfile1, "wb");

    return size;
}

JNIEXPORT void JNICALL
Java_com_ffmpeg_AudioManager_encodeAAC(JNIEnv *env, jobject instance, jbyteArray bytes_, jint length) {
    jbyte *bytes = (*env)->GetByteArrayElements(env, bytes_, NULL);
    LOGE("length = %d", length);

    int outLength = length;
    char *out = calloc(outLength , sizeof(char));
    LOGE("outLength1 = %d %d ", (length * sizeof(char)), bytes[0]);

    int len = aac_encode_audio(bytes, length, out, outLength);

    LOGE("len = %d %d %d %d", len, bytes[0], bytes[1], bytes[2]);
    fwrite(out, 1, len, out1);

    free(out);

    (*env)->ReleaseByteArrayElements(env, bytes_, bytes, 0);
}

JNIEXPORT void JNICALL
Java_com_ffmpeg_AudioManager_release(JNIEnv *env, jobject instance) {
    aac_close();
    fclose(out1);

    simplest_aac_parser(outfile1);
}