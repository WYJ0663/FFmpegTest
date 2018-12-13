//
// Created by yijunwu on 2018/12/7.
//

#ifndef FFMPEGTEST_FDK_AAC_TEST_H
#define FFMPEGTEST_FDK_AAC_TEST_H

#include <fdk-aac/aacenc_lib.h>
#include "../Log.h"

HANDLE_AACENCODER handle;
int channels;
int sampleRate;
int bitRate;

void aac_init_param(int _channels, int _sampleRate, int _bitRate) {
    channels = _channels;
    sampleRate = _sampleRate;
    bitRate = _bitRate;
}


/**
 * 初始化fdk-aac的参数，设置相关接口使得
 * @return
 */
int aac_init() {
    LOGE("channels sampleRate bitRate %d %d %d",channels,sampleRate,bitRate);
    //打开AAC音频编码引擎，创建AAC编码句柄
    if (aacEncOpen(&handle, 0, channels) != AACENC_OK) {
        LOGE("Unable to open fdkaac encoder\n");
        return -1;
    }

    // AACENC_AOT设置为aac lc
    if (aacEncoder_SetParam(handle, AACENC_AOT, 2) != AACENC_OK) {
        LOGE("Unable to set the AOT\n");
        return -1;
    }

    if (aacEncoder_SetParam(handle, AACENC_SAMPLERATE, sampleRate) != AACENC_OK) {
        LOGE("Unable to set the sampleRate\n");
        return -1;
    }

    // AACENC_CHANNELMODE设置为双通道
    if (aacEncoder_SetParam(handle, AACENC_CHANNELMODE, MODE_2) != AACENC_OK) {
        LOGE("Unable to set the channel mode\n");
        return -1;
    }

    if (aacEncoder_SetParam(handle, AACENC_CHANNELORDER, 1) != AACENC_OK) {
        LOGE("Unable to set the wav channel order\n");
        return 1;
    }
    if (aacEncoder_SetParam(handle, AACENC_BITRATE, bitRate) != AACENC_OK) {
        LOGE("Unable to set the bitrate\n");
        return -1;
    }
    if (aacEncoder_SetParam(handle, AACENC_TRANSMUX, 0) != AACENC_OK) { //0-raw 2-adts
        LOGE("Unable to set the ADTS transmux\n");
        return -1;
    }

    if (aacEncoder_SetParam(handle, AACENC_AFTERBURNER, 1) != AACENC_OK) {
        LOGE("Unable to set the ADTS AFTERBURNER\n");
        return -1;
    }

    if (aacEncEncode(handle, NULL, NULL, NULL, NULL) != AACENC_OK) {
        LOGE("Unable to initialize the encoder\n");
        return -1;
    }

    AACENC_InfoStruct info = {0};
    if (aacEncInfo(handle, &info) != AACENC_OK) {
        LOGE("Unable to get the encoder info\n");
        return -1;
    }

    //返回数据给上层，表示每次传递多少个数据最佳，这样encode效率最高
    int inputSize = channels * 2 * info.frameLength;
    LOGE("inputSize = %d", inputSize);

    return inputSize;
}

//static INT_PCM inputBuffer[8*2048];
//static UCHAR ancillaryBuffer[50];
//static AACENC_MetaData metaDataSetup;
//static UCHAR outputBuffer[8192];

int aac_encode_audio2( unsigned char *inBytes, int length, unsigned char *outBytes, int outLength) {
    void * inBuffer[] = { inBytes };
     INT inBufferIds[] = { IN_AUDIO_DATA };
     INT inBufferSize[] = { length };
     INT inBufferElSize[] = { sizeof(INT_PCM), };

    void * outBuffer[] = { outBytes };
     INT outBufferIds[] = { OUT_BITSTREAM_DATA };
     INT outBufferSize[] = { length };
     INT outBufferElSize[] = { sizeof(UCHAR) };


    AACENC_BufDesc inBufDesc;
    AACENC_BufDesc outBufDesc;

    inBufDesc.numBufs = 1;
    inBufDesc.bufs = (void ** )&inBuffer;
    inBufDesc.bufferIdentifiers = inBufferIds;
    inBufDesc.bufSizes = inBufferSize;
    inBufDesc.bufElSizes = inBufferElSize;

    outBufDesc.numBufs = 1;
    outBufDesc.bufs = (void ** )&outBuffer;
    outBufDesc.bufferIdentifiers = outBufferIds;
    outBufDesc.bufSizes = outBufferSize;
    outBufDesc.bufElSizes = outBufferElSize;

    AACENC_InArgs inargs;
    AACENC_OutArgs outargs;
    inargs.numInSamples = length / 2;
    AACENC_ERROR err = aacEncEncode(handle, &inBufDesc, &outBufDesc, &inargs, &outargs);

    LOGE("all %d %d ", err, outargs.numOutBytes);

    if (err != AACENC_OK) {
        LOGE("Encoding aac failed %d %d ", err, outargs.numOutBytes);
        return err;
    }
    return outargs.numOutBytes;
}
/**
 * Fdk-AAC库压缩裸音频PCM数据，转化为AAC，这里为什么用fdk-aac，这个库相比普通的aac库，压缩效率更高
 * @param inBytes
 * @param length
 * @param outBytes
 * @param outLength
 * @return
 */
int aac_encode_audio(unsigned char *inBytes, int length, unsigned char *outBytes, int outLength) {
    void *in_ptr, *out_ptr;
    AACENC_BufDesc in_buf = {0};
    int in_identifier = IN_AUDIO_DATA;
    int in_elem_size = 2;
    //传递input数据给in_buf
    in_ptr = inBytes;
    in_buf.bufs = &in_ptr;
    in_buf.numBufs = 1;
    in_buf.bufferIdentifiers = &in_identifier;
    in_buf.bufSizes = &length;
    in_buf.bufElSizes = &in_elem_size;

    AACENC_BufDesc out_buf = {0};
    int out_identifier = OUT_BITSTREAM_DATA;
    int elSize = 1;
    //out数据放到out_buf中
    out_ptr = outBytes;
    out_buf.bufs = &out_ptr;
    out_buf.numBufs = 1;
    out_buf.bufferIdentifiers = &out_identifier;
    out_buf.bufSizes = &outLength;
    out_buf.bufElSizes = &elSize;

    AACENC_InArgs in_args = {0};
    in_args.numInSamples = length / 2;  //size为pcm字节数

    AACENC_OutArgs out_args = {0};
    AACENC_ERROR err;

    //利用aacEncEncode来编码PCM裸音频数据，上面的代码都是fdk-aac的流程步骤
    if ((err = aacEncEncode(handle, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK) {
        LOGE("Encoding aac failed\n err=%d",err);
        return err;
    }
    LOGE("all err=%d", err);

    //返回编码后的有效字段长度
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
