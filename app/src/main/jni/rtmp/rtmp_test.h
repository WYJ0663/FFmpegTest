//
// Created by yijunwu on 2018/12/3.
//

#ifndef FFMPEGTEST_RTMP_H
#define FFMPEGTEST_RTMP_H

void rtmp_init(unsigned char *url);

void pushSPSPPS(char *sps, int spsLen, char *pps, int ppsLen);

void pushVideoData(char *data, int dataLen, int keyFrame);

void pushAudioData(char *data, int dataLen);

#endif //FFMPEGTEST_RTMP_H
