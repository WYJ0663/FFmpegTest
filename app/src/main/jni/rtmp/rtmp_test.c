//
// Created by yijunwu on 2018/12/3.
//

#include "rtmp_test.h"
#include "../Log.h"
#include <librtmp/rtmp.h>
#include <malloc.h>
#include <memory.h>

RTMP *rtmp;
uint32_t start_time;

void rtmp_init(unsigned char *url) {
    rtmp = RTMP_Alloc();
    RTMP_Init(rtmp);

    rtmp->Link.timeout = 5;
    RTMP_SetupURL(rtmp, (char *) url);
    RTMP_EnableWrite(rtmp);

    if (!RTMP_Connect(rtmp, NULL)) {
        LOGE("RTMP_Connect error");
    } else {
        LOGE("RTMP_Connect success.");
    }

    if (!RTMP_ConnectStream(rtmp, 0)) {
        LOGE("RTMP_ConnectStream error");
    } else {
        LOGE("RTMP_ConnectStream success.");
    }
    start_time = RTMP_GetTime();
    LOGE(" start_time = %d", start_time);

}

void pushSPSPPS(char *sps, int spsLen, char *pps, int ppsLen) {
    int bodySize = spsLen + ppsLen + 16;
    RTMPPacket *rtmpPacket = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    RTMPPacket_Alloc(rtmpPacket, bodySize);
    RTMPPacket_Reset(rtmpPacket);

    char *body = rtmpPacket->m_body;

    int i = 0;
    //frame type(4bit)和CodecId(4bit)合成一个字节(byte)
    //frame type 关键帧1  非关键帧2
    //CodecId  7表示avc
    body[i++] = 0x17;

    //fixed 4byte
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;

    //configurationVersion： 版本 1byte
    body[i++] = 0x01;

    //AVCProfileIndication：Profile 1byte  sps[1]
    body[i++] = sps[1];

    //compatibility：  兼容性 1byte  sps[2]
    body[i++] = sps[2];

    //AVCLevelIndication： ProfileLevel 1byte  sps[3]
    body[i++] = sps[3];

    //lengthSizeMinusOne： 包长数据所使用的字节数  1byte
    body[i++] = 0xff;

    //sps个数 1byte
    body[i++] = 0xe1;
    //sps长度 2byte
    body[i++] = (spsLen >> 8) & 0xff;
    body[i++] = spsLen & 0xff;

    //sps data 内容
    memcpy(&body[i], sps, spsLen);
    i += spsLen;
    //pps个数 1byte
    body[i++] = 0x01;
    //pps长度 2byte
    body[i++] = (ppsLen >> 8) & 0xff;
    body[i++] = ppsLen & 0xff;
    //pps data 内容
    memcpy(&body[i], pps, ppsLen);


    rtmpPacket->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    rtmpPacket->m_nBodySize = bodySize;
    rtmpPacket->m_nTimeStamp = 0;
    rtmpPacket->m_hasAbsTimestamp = 0;
    rtmpPacket->m_nChannel = 0x04;//音频或者视频
    rtmpPacket->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    rtmpPacket->m_nInfoField2 = rtmp->m_stream_id;

    /*发送*/
    int nRet = 0;
    if (RTMP_IsConnected(rtmp)) {
        nRet = RTMP_SendPacket(rtmp, rtmpPacket, TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
    }
    /*释放内存*/
    free(rtmpPacket);

    LOGE("pushSPSPPS %d", nRet);
}


void pushVideoData(char *data, int dataLen, int keyFrame) {

    int bodySize = dataLen + 9;
    RTMPPacket *rtmpPacket = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    RTMPPacket_Alloc(rtmpPacket, bodySize);
    RTMPPacket_Reset(rtmpPacket);

    char *body = rtmpPacket->m_body;

    int i = 0;
    //frame type(4bit)和CodecId(4bit)合成一个字节(byte)
    //frame type 关键帧1  非关键帧2
    //CodecId  7表示avc
    if (keyFrame) {
        body[i++] = 0x17;
    } else {
        body[i++] = 0x27;
    }

    //fixed 4byte   0x01表示NALU单元
    body[i++] = 0x01;
    body[i++] = 0x00;
    body[i++] = 0x00;
    body[i++] = 0x00;

    //dataLen  4byte
    body[i++] = (dataLen >> 24) & 0xff;
    body[i++] = (dataLen >> 16) & 0xff;
    body[i++] = (dataLen >> 8) & 0xff;
    body[i++] = dataLen & 0xff;

    //data
    memcpy(&body[i], data, dataLen);

    rtmpPacket->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    rtmpPacket->m_nBodySize = bodySize;
    //持续播放时间
    rtmpPacket->m_nTimeStamp = RTMP_GetTime() - start_time;
    //进入直播播放开始时间
    rtmpPacket->m_hasAbsTimestamp = 0;
    rtmpPacket->m_nChannel = 0x04;//音频或者视频
    rtmpPacket->m_headerType = RTMP_PACKET_SIZE_LARGE;
    rtmpPacket->m_nInfoField2 = rtmp->m_stream_id;

    /*发送*/
    int nRet = 0;
    if (RTMP_IsConnected(rtmp)) {
        nRet = RTMP_SendPacket(rtmp, rtmpPacket, TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
    }
    /*释放内存*/
    free(rtmpPacket);

    LOGE("pushVideoData %d", nRet);
}

void pushAudioData(char *data, int dataLen) {
    int bodySize = dataLen + 2;
    RTMPPacket *rtmpPacket = (RTMPPacket *) malloc(sizeof(RTMPPacket));
    RTMPPacket_Alloc(rtmpPacket, bodySize);
    RTMPPacket_Reset(rtmpPacket);

    char *body = rtmpPacket->m_body;
    //前四位表示音频数据格式  10（十进制）表示AAC，16进制就是A
    //第5-6位的数值表示采样率，0 = 5.5 kHz，1 = 11 kHz，2 = 22 kHz，3(11) = 44 kHz。
    //第7位表示采样精度，0 = 8bits，1 = 16bits。
    //第8位表示音频类型，0 = mono，1 = stereo
    //这里是44100 立体声 16bit 二进制就是1111   16进制就是F
    body[0] = 0xAF;

    //0x00 aac头信息,  0x01 aac 原始数据
    //这里都用0x01都可以
    body[1] = 0x01;

    //data
    memcpy(&body[2], data, dataLen);

    rtmpPacket->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    rtmpPacket->m_nBodySize = bodySize;
    //持续播放时间
    rtmpPacket->m_nTimeStamp = RTMP_GetTime() - start_time;
    //进入直播播放开始时间
    rtmpPacket->m_hasAbsTimestamp = 0;
    rtmpPacket->m_nChannel = 0x04;//音频或者视频
    rtmpPacket->m_headerType = RTMP_PACKET_SIZE_LARGE;
    rtmpPacket->m_nInfoField2 = rtmp->m_stream_id;

    /*发送*/
    int nRet = 0;
    if (RTMP_IsConnected(rtmp)) {
        nRet = RTMP_SendPacket(rtmp, rtmpPacket, TRUE); /*TRUE为放进发送队列,FALSE是不放进发送队列,直接发送*/
    }
    /*释放内存*/
    free(rtmpPacket);

//    LOGE("pushAudioData %d", nRet);
}


void release() {
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
}