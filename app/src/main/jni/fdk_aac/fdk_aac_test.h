//
// Created by yijunwu on 2018/12/7.
//

#ifndef FFMPEGTEST_FDK_AAC_TEST_H
#define FFMPEGTEST_FDK_AAC_TEST_H

#include <fdk-aac/aacenc_lib.h>
#include "../Log.h"

HANDLE_AACENCODER handle;
int channels;
int sample_rate;
int bitRate = 64000;;

void aac_init_param(int _channels, int _sampleRate, int _bitRate) {
    channels = _channels;
    sample_rate = _sampleRate;
    bitRate = _bitRate;
}

/**
 * 初始化fdk-aac的参数，设置相关接口使得
 * @return
 */
int aac_init() {
    int aot = 2;
    int afterburner = 1;
    int eld_sbr = 0;
    int vbr = 0;
    CHANNEL_MODE mode = MODE_1;
    AACENC_InfoStruct info = {0};
    LOGE("channels sample_rate bitrate %d %d %d", channels, sample_rate, bitRate);
    switch (channels) {
        case 1:
            mode = MODE_1;
            break;
        case 2:
            mode = MODE_2;
            break;
        case 3:
            mode = MODE_1_2;
            break;
        case 4:
            mode = MODE_1_2_1;
            break;
        case 5:
            mode = MODE_1_2_2;
            break;
        case 6:
            mode = MODE_1_2_2_1;
            break;
        default:
            LOGE("Unsupported WAV channels %d\n", channels);
            return 1;
    }
    if (aacEncOpen(&handle, 0, channels) != AACENC_OK) {
        LOGE("Unable to open encoder\n");
        return 1;
    }
    if (aacEncoder_SetParam(handle, AACENC_AOT, aot) != AACENC_OK) {
        LOGE("Unable to set the AOT\n");
        return 1;
    }
    if (aot == 39 && eld_sbr) {
        if (aacEncoder_SetParam(handle, AACENC_SBR_MODE, 1) != AACENC_OK) {
            LOGE("Unable to set SBR mode for ELD\n");
            return 1;
        }
    }
    if (aacEncoder_SetParam(handle, AACENC_SAMPLERATE, sample_rate) != AACENC_OK) {
        LOGE("Unable to set the AOT\n");
        return 1;
    }
    if (aacEncoder_SetParam(handle, AACENC_CHANNELMODE, mode) != AACENC_OK) {
        LOGE("Unable to set the channel mode\n");
        return 1;
    }
    if (aacEncoder_SetParam(handle, AACENC_CHANNELORDER, 1) != AACENC_OK) {
        LOGE("Unable to set the wav channel order\n");
        return 1;
    }
    if (vbr) {
        if (aacEncoder_SetParam(handle, AACENC_BITRATEMODE, vbr) != AACENC_OK) {
            LOGE("Unable to set the VBR bitrate mode\n");
            return 1;
        }
    } else {
        if (aacEncoder_SetParam(handle, AACENC_BITRATE, bitRate) != AACENC_OK) {
            LOGE("Unable to set the bitrate\n");
            return 1;
        }
    }
    if (aacEncoder_SetParam(handle, AACENC_TRANSMUX, TT_MP4_ADTS) != AACENC_OK) {
        LOGE("Unable to set the ADTS transmux\n");
        return 1;
    }
    if (aacEncoder_SetParam(handle, AACENC_AFTERBURNER, afterburner) != AACENC_OK) {
        LOGE("Unable to set the afterburner mode\n");
        return 1;
    }
    if (aacEncEncode(handle, NULL, NULL, NULL, NULL) != AACENC_OK) {
        LOGE("Unable to initialize the encoder\n");
        return 1;
    }
    if (aacEncInfo(handle, &info) != AACENC_OK) {
        LOGE("Unable to get the encoder info\n");
        return 1;
    }

    //返回数据给上层，表示每次传递多少个数据最佳，这样encode效率最高
    int inputSize = channels * 2 * info.frameLength;
    LOGE("inputSize = %d", inputSize);

    return inputSize;
}

/**
 * Fdk-AAC库压缩裸音频PCM数据，转化为AAC，这里为什么用fdk-aac，这个库相比普通的aac库，压缩效率更高
 * @param inBytes
 * @param length
 * @param outBytes
 * @param outLength
 * @return
 */
int aac_encode_audio(unsigned char *convert_buf, int read, unsigned char *outbuf, int outLength) {
    AACENC_BufDesc in_buf = {0}, out_buf = {0};
    AACENC_InArgs in_args = {0};
    AACENC_OutArgs out_args = {0};
    int in_identifier = IN_AUDIO_DATA;
    int in_size, in_elem_size;
    int out_identifier = OUT_BITSTREAM_DATA;
    int out_size, out_elem_size;
    void *in_ptr, *out_ptr;

    in_ptr = convert_buf;
    in_size = read;
    in_elem_size = 2;

    in_args.numInSamples = read <= 0 ? -1 : read / 2;
    in_buf.numBufs = 1;
    in_buf.bufs = &in_ptr;
    in_buf.bufferIdentifiers = &in_identifier;
    in_buf.bufSizes = &in_size;
    in_buf.bufElSizes = &in_elem_size;

    out_ptr = outbuf;
//    out_size = sizeof(outbuf);
    out_size = outLength;
    out_elem_size = 1;
    out_buf.numBufs = 1;
    out_buf.bufs = &out_ptr;
    out_buf.bufferIdentifiers = &out_identifier;
    out_buf.bufSizes = &out_size;
    out_buf.bufElSizes = &out_elem_size;

    AACENC_ERROR err;
    if ((err = aacEncEncode(handle, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK) {
        if (err == AACENC_ENCODE_EOF)
        LOGE("Encoding failed\n");
        return 0;
    }
    LOGE("numOutBytes %d", out_args.numOutBytes);

    return out_args.numOutBytes;
}

int aac_close() {
    if (handle) {
        aacEncClose(&handle);
        handle = NULL;
    }
    return 1;
}

#endif //FFMPEGTEST_FDK_AAC_TEST_H
