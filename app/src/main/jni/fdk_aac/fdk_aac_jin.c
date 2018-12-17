//
// Created by yijunwu on 2018/12/10.
//


#include <jni.h>
#include <malloc.h>
#include <memory.h>
#include "fdk_aac_test.h"
#include "acc.h"
#include "acc_enc.h"

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
    int16_t *convert_buf = (int16_t *) malloc(length);
    for (int i = 0; i < length / 2; i++) {
        const uint8_t *in = &bytes[2 * i];
        convert_buf[i] = in[0] | (in[1] << 8);
    }

    int outLength = length;
    uint8_t *out = malloc(outLength);
    LOGE("outLength1 %d", outLength);

    int len = 0;
    len = aac_encode_audio(convert_buf, length, out, outLength);

    LOGE("len %d", len);
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

JNIEXPORT void JNICALL
Java_com_ffmpeg_AudioManager_changeWav2AAC(JNIEnv *env, jclass type) {
    aac_main();
}
