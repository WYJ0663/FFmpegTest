//
//推流h264
// Created by yijunwu on 2018/12/4.
//

#ifndef FFMPEGTEST_H264_H
#define FFMPEGTEST_H264_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Log.h"
#include "rtmp_test.h"
#include <unistd.h> // sleep 的头文件

typedef enum {
    NALU_TYPE_SLICE = 1,
    NALU_TYPE_DPA = 2,
    NALU_TYPE_DPB = 3,
    NALU_TYPE_DPC = 4,
    NALU_TYPE_IDR = 5,
    NALU_TYPE_SEI = 6,
    NALU_TYPE_SPS = 7,
    NALU_TYPE_PPS = 8,
    NALU_TYPE_AUD = 9,
    NALU_TYPE_EOSEQ = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL = 12,
} NaluType;

typedef enum {
    NALU_PRIORITY_DISPOSABLE = 0,
    NALU_PRIRITY_LOW = 1,
    NALU_PRIORITY_HIGH = 2,
    NALU_PRIORITY_HIGHEST = 3
} NaluPriority;


typedef struct {
    int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
    unsigned max_size;            //! Nal Unit Buffer size
    int forbidden_bit;            //! should be always FALSE
    int nal_reference_idc;        //! NALU_PRIORITY_xxxx
    int nal_unit_type;            //! NALU_TYPE_xxxx
    char *buf;                    //! contains the first byte followed by the EBSP
} NALU_t;

FILE *h264bitstream = NULL;                //!< the bit stream file

int info2 = 0, info3 = 0;

static int FindStartCode2(unsigned char *Buf) {
    if (Buf[0] != 0 || Buf[1] != 0 || Buf[2] != 1) return 0; //0x000001?
    else return 1;
}

static int FindStartCode3(unsigned char *Buf) {
    if (Buf[0] != 0 || Buf[1] != 0 || Buf[2] != 0 || Buf[3] != 1) return 0;//0x00000001?
    else return 1;
}

int GetAnnexbNALU(NALU_t *nalu) {
    int pos = 0;
    int StartCodeFound, rewind;
    unsigned char *Buf;

    if ((Buf = (unsigned char *) calloc(nalu->max_size, sizeof(char))) == NULL)
        LOGE("GetAnnexbNALU: Could not allocate Buf memory\n");

    nalu->startcodeprefix_len = 3;

    if (3 != fread(Buf, 1, 3, h264bitstream)) {//读3个字节
        free(Buf);
        return 0;
    }
    info2 = FindStartCode2(Buf);
    if (info2 != 1) {//不是0x000001
        if (1 != fread(Buf + 3, 1, 1, h264bitstream)) {//多读一个字节
            free(Buf);
            return 0;
        }
        info3 = FindStartCode3(Buf);
        if (info3 != 1) {//不是0x00000001，结束
            free(Buf);
            return -1;
        } else {
            pos = 4;
            nalu->startcodeprefix_len = 4;//前缀长度4
        }
    } else {
        nalu->startcodeprefix_len = 3;//前缀长度3
        pos = 3;
    }
    StartCodeFound = 0;
    info2 = 0;
    info3 = 0;

    while (!StartCodeFound) {
        if (feof(h264bitstream)) {//文件结束
            nalu->len = (pos - 1) - nalu->startcodeprefix_len;//-1？不是
            memcpy(nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
            nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
            nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
            nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
            free(Buf);
            return pos - 1;
        }
        Buf[pos++] = fgetc(h264bitstream);//读一个字节
        info3 = FindStartCode3(&Buf[pos - 4]);
        if (info3 != 1)//是不是0x00000001
            info2 = FindStartCode2(&Buf[pos - 3]);//是不是0x000001
        StartCodeFound = (info2 == 1 || info3 == 1);//两张
    }

    // Here, we have found another start code (and read length of startcode bytes more than we should
    // have.  Hence, go back in the file
    rewind = (info3 == 1) ? -4 : -3;

    if (0 != fseek(h264bitstream, rewind, SEEK_CUR)) {//文件位置为给定的偏移
        free(Buf);
        LOGE("GetAnnexbNALU: Cannot fseek in the bit stream file");
    }

    // Here the Start code, the complete NALU, and the next start code is in the Buf.
    // The size of Buf is pos, pos+rewind are the number of bytes excluding the next
    // start code, and (pos+rewind)-startcodeprefix_len is the size of the NALU excluding the start code

    nalu->len = (pos + rewind) - nalu->startcodeprefix_len;
    memcpy(nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//
    nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit          1000 0000
    nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit     0110 0000
    nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit        0001 1111
    free(Buf);

    return (pos + rewind);
}

NALU_t *SPS;
NALU_t *PPS;

NALU_t *copyNALU(NALU_t *src) {
    if (src == 0) {
        return 0;
    }

    NALU_t *n = 0;
    n = (NALU_t *) calloc(1, sizeof(NALU_t));
    if (n == NULL) {
        LOGE("copyNALU Alloc NALU Error\n");
        return 0;
    }

    n->buf = (char *) calloc(src->len, sizeof(char));
    if (n->buf == NULL) {
        free(n);
        LOGE("copyNALU AllocNALU: n->buf");
        return 0;
    }

    //copy
    n->startcodeprefix_len = src->startcodeprefix_len;
    n->len = src->len;
    n->max_size = src->max_size;
    n->forbidden_bit = src->forbidden_bit;
    n->nal_reference_idc = src->forbidden_bit;
    n->nal_unit_type = src->nal_unit_type;
    memcpy(n->buf, src->buf, src->len);
    return n;
}

/**
 * Analysis H.264 Bitstream
 * @param url    Location of input H.264 bitstream file.
 */
int simplest_h264_parser(char *url) {
    LOGE("url %s ", url);
    NALU_t *n;
    int buffersize = 100000;

    //FILE *myout=fopen("output_log.txt","wb+");
    FILE *myout = stdout;

    h264bitstream = fopen(url, "rb+");
    if (h264bitstream == NULL) {
        LOGE("Open file error\n");
        return 0;
    }

    n = (NALU_t *) calloc(1, sizeof(NALU_t));
    if (n == NULL) {
        LOGE("Alloc NALU Error\n");
        return 0;
    }

    n->max_size = buffersize;
    n->buf = (char *) calloc(buffersize, sizeof(char));
    if (n->buf == NULL) {
        free(n);
        LOGE("AllocNALU: n->buf");
        return 0;
    }

    int data_offset = 0;
    int nal_num = 0;
//    LOGE("-----+-------- NALU Table ------+---------+\n");
//    LOGE(" NUM |    POS  |    IDC |  TYPE |   LEN   |\n");
//    LOGE("-----+---------+--------+-------+---------+\n");

    while (!feof(h264bitstream)) {
        int data_lenth;
        data_lenth = GetAnnexbNALU(n);

        char type_str[20] = {0};
        switch (n->nal_unit_type) {
            case NALU_TYPE_SLICE:
                sprintf(type_str, "SLICE");//P
                pushVideoData(n->buf, n->len, 0);
                usleep(20 * 1000);
                LOGE("push P data")
                break;
            case NALU_TYPE_DPA:
                sprintf(type_str, "DPA");
                break;
            case NALU_TYPE_DPB:
                sprintf(type_str, "DPB");
                break;
            case NALU_TYPE_DPC:
                sprintf(type_str, "DPC");
                break;
            case NALU_TYPE_IDR:
                sprintf(type_str, "IDR");//I
                pushSPSPPS(SPS->buf, SPS->len, PPS->buf, PPS->len);
                pushVideoData(n->buf, n->len, 1);
                usleep(20 * 1000);
                LOGE("push I data")
                break;
            case NALU_TYPE_SEI:
                sprintf(type_str, "SEI");
                break;
            case NALU_TYPE_SPS:
                sprintf(type_str, "SPS");
                SPS = copyNALU(n);
                break;
            case NALU_TYPE_PPS:
                sprintf(type_str, "PPS");
                PPS = copyNALU(n);
                break;
            case NALU_TYPE_AUD:
                sprintf(type_str, "AUD");
                break;
            case NALU_TYPE_EOSEQ:
                sprintf(type_str, "EOSEQ");
                break;
            case NALU_TYPE_EOSTREAM:
                sprintf(type_str, "EOSTREAM");
                break;
            case NALU_TYPE_FILL:
                sprintf(type_str, "FILL");
                break;
        }
        char idc_str[20] = {0};
        switch (n->nal_reference_idc >> 5) {
            case NALU_PRIORITY_DISPOSABLE:
                sprintf(idc_str, "DISPOS");
                break;
            case NALU_PRIRITY_LOW:
                sprintf(idc_str, "LOW");
                break;
            case NALU_PRIORITY_HIGH:
                sprintf(idc_str, "HIGH");
                break;
            case NALU_PRIORITY_HIGHEST:
                sprintf(idc_str, "HIGHEST");
                break;
        }

//        LOGE("%5d| %8d| %7s| %6s| %8d|\n", nal_num, data_offset, idc_str, type_str, n->len);

        data_offset = data_offset + data_lenth;

        nal_num++;
    }
    //Free
    if (n) {
        if (n->buf) {
            free(n->buf);
            n->buf = NULL;
        }
        free(n);
    }
    return 0;
}

#endif //FFMPEGTEST_H264_H
