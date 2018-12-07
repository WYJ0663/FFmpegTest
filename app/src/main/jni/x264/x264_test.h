//
// Created by yijunwu on 2018/12/5.
//

#ifndef FFMPEGTEST_X264_TEST_H
#define FFMPEGTEST_X264_TEST_H

#include <malloc.h>
#include <memory.h>
#include "libx264/x264.h"
#include "../Log.h"

x264_picture_t *pic_in;
x264_picture_t *pic_out;
x264_param_t params;
x264_nal_t *nals;
x264_t *encoder;
int num_nals = 0;

//原图的宽高
int in_width = 0;
int in_height = 0;
int bitrate = 128;
int in_orientation = 0;

void NV21ToI420(char *dstData, char *srcdata, int len) {

    int size = len * 4 / 6;//NV12:YYYYVU
    // Y
    memcpy(dstData, srcdata, len * 4 / 6);
    for (int i = 0; i < size / 4; i++) {
        dstData[size + i] = srcdata[size + i * 2 + 1]; //U
        dstData[size + size / 4 + i] = srcdata[size + i * 2]; //V
    }
}

void YUV420spRotateNegative90(char *dst, const char *src, int width, int height) {
    int nWidth = 0, nHeight = 0;
    int wh = 0;
    int uvHeight = 0;
    if (width != nWidth || height != nHeight) {
        nWidth = width;
        nHeight = height;
        wh = width * height;
        uvHeight = height >> 1;//uvHeight = height / 2
    }

    //旋转Y
    int k = 0;
    for (int i = 0; i < width; i++) {
        int nPos = width - 1;
        for (int j = 0; j < height; j++) {
            dst[k] = src[nPos - i];
            k++;
            nPos += width;
        }
    }

    for (int i = 0; i < width; i += 2) {
        int nPos = wh + width - 1;
        for (int j = 0; j < uvHeight; j++) {
            dst[k] = src[nPos - i - 1];
            dst[k + 1] = src[nPos - i];
            k += 2;
            nPos += width;
        }
    }
    return;
}

void rotate90(int start, int width, int height, char *dst, char *src) {
    int wh = width * height;
    int k = start;
    for (int i = 0; i < width; i++) {
        int nPos = wh - width + i + start;
        for (int j = 0; j < height; j++) {
            dst[k] = src[nPos];
            k++;
            nPos -= width;
        }
    }
}

void YUV420Rotate90(char *dst, char *src, int width, int height) {

    int wh = width * height;
    int uvWidth = width >> 1;
    int uvHeight = height >> 1;

    rotate90(0, width, height, dst, src);
    rotate90(wh, uvWidth, uvHeight, dst, src);
    rotate90(wh * 5 / 4, uvWidth, uvHeight, dst, src);

    return;
}

void setParams() {
    //preset
    //默认：medium
    //一些在压缩效率和运算时间中平衡的预设值。如果指定了一个预设值，它会在其它选项生效前生效。
    //可选：ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow and placebo.
    //建议：可接受的最慢的值
    //tune
    //默认：无
    //说明：在上一个选项基础上进一步优化输入。如果定义了一个tune值，它将在preset之后，其它选项之前生效。
    //可选：film, animation, grain, stillimage, psnr, ssim, fastdecode, zerolatency and touhou.
    //建议：根据输入选择。如果没有合适的就不要指定。
    //后来发现设置x264_param_default_preset(&param, "fast" , "zerolatency" );后就能即时编码了
    x264_param_default_preset(&params, "veryfast", "zerolatency");

    //I帧间隔
    params.i_csp = X264_CSP_I420;
    if (in_orientation == 90 || in_orientation == 270) {
        params.i_width = in_height;
        params.i_height = in_width;
    } else {
        params.i_width = in_width;
        params.i_height = in_height;
    }


    //并行编码多帧
    params.i_threads = X264_SYNC_LOOKAHEAD_AUTO;
    params.i_fps_num = 25;//getFps();
    params.i_fps_den = 1;

    // B frames 两个相关图像间B帧的数目 */
    params.i_bframe = 5;//getBFrameFrq();
    params.b_sliced_threads = 1;
    params.b_vfr_input = 0;
    params.i_timebase_num = params.i_fps_den;
    params.i_timebase_den = params.i_fps_num;

    // Intra refres:
    params.i_keyint_max = 50;
    params.i_keyint_min = 1;
    params.b_intra_refresh = 1;

    //参数i_rc_method表示码率控制，CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
    //平均码率
    params.rc.i_rc_method = X264_RC_ABR;
    //图像质量控制,rc.f_rf_constant是实际质量，越大图像越花，越小越清晰
    //param.rc.f_rf_constant_max ，图像质量的最大值
    params.rc.f_rf_constant = 25;
    params.rc.f_rf_constant_max = 35;

    // For streaming:
    //* 码率(比特率,单位Kbps)x264使用的bitrate需要/1000
    params.rc.i_bitrate = bitrate / 1000;
    //瞬时最大码率,平均码率模式下，最大瞬时码率，默认0(与-B设置相同)
    params.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2;
    params.b_repeat_headers = 0;// 重复SPS/PPS 放到关键帧前面
    params.b_annexb = 1;//if set, place start codes (4 bytes) before NAL units,

    //是否把SPS和PPS放入每一个关键帧
    //SPS Sequence Parameter Set 序列参数集，PPS Picture Parameter Set 图像参数集
    //为了提高图像的纠错能力,该参数设置是让每个I帧都附带sps/pps。
    //param.b_repeat_headers = 1;
    //设置Level级别,编码复杂度
    params.i_level_idc = 30;

    //profile
    //默认：无
    //说明：限制输出文件的profile。这个参数将覆盖其它所有值，此选项能保证输出profile兼容的视频流。如果使用了这个选项，将不能进行无损压缩（qp 0 or crf 0）。
    //可选：baseline，main，high
    //建议：不设置。除非解码环境只支持main或者baseline profile的解码。
    x264_param_apply_profile(&params, "baseline");
}


int x264_init(int mWidth, int mHeight, int bitrate, int orientation) {
    in_width = mWidth;
    in_height = mHeight;
    in_orientation = orientation;
    LOGE("video encoder setting");

    int r = 0;
    int nheader = 0;
    int header_size = 0;

    if (encoder) {
        LOGE("Already opened. first call close()");
        return 0;
    }

    // set encoder parameters
    setParams();
    //按照色度空间分配内存，即为图像结构体x264_picture_t分配内存，并返回内存的首地址作为指针
    //i_csp(图像颜色空间参数，目前只支持I420/YUV420)为X264_CSP_I420
    pic_in = (x264_picture_t *) malloc(sizeof(x264_picture_t));
    pic_out = (x264_picture_t *) malloc(sizeof(x264_picture_t));

    x264_picture_alloc(pic_in, params.i_csp, params.i_width, params.i_height);
    //create the encoder using our params 打开编码器
    encoder = x264_encoder_open(&params);

    if (!encoder) {
        LOGE("Cannot open the encoder");
//        x264_release();
        return 0;
    }

    // write headers
    r = x264_encoder_headers(encoder, &nals, &nheader);
    if (r < 0) {
        LOGE("x264_encoder_headers() failed");
        return 0;
    }

}

//编码h264帧
int encodeFrame(char *inBytes, int pts, char *outBytes, int *outFrameSize) {

    //YUV420P数据转化为h264
    int i420_y_size = in_width * in_height;
    int i420_u_size = (in_width >> 1) * (in_height >> 1);
    int i420_v_size = i420_u_size;

    uint8_t *i420_y_data = (uint8_t *) inBytes;
    uint8_t *i420_u_data = (uint8_t *) inBytes + i420_y_size;
    uint8_t *i420_v_data = (uint8_t *) inBytes + i420_y_size + i420_u_size;
    //将Y,U,V数据保存到pic_in.img的对应的分量中，还有一种方法是用AV_fillPicture和sws_scale来进行变换
    memcpy(pic_in->img.plane[0], i420_y_data, i420_y_size);
    memcpy(pic_in->img.plane[1], i420_u_data, i420_u_size);
    memcpy(pic_in->img.plane[2], i420_v_data, i420_v_size);
    LOGE("encodeFrame data");
    // and encode and store into pic_out
    pic_in->i_pts = pts;
    //最主要的函数，x264编码，pic_in为x264输入，pic_out为x264输出
    int frame_size = x264_encoder_encode(encoder, &nals, &num_nals, pic_in, pic_out);
    LOGE("encodeFrame data %d num_nals %d", frame_size, num_nals);
    if (frame_size) {
        /*Here first four bytes proceeding the nal unit indicates frame length*/
        int have_copy = 0;
        //编码后，h264数据保存为nal了，我们可以获取到nals[i].type的类型判断是sps还是pps
        //或者是否是关键帧，nals[i].i_payload表示数据长度，nals[i].p_payload表示存储的数据
        //编码后，我们按照nals[i].i_payload的长度来保存copy h264数据的，然后抛给java端用作
        //rtmp发送数据，outFrameSize是变长的，当有sps pps的时候大于1，其它时候值为1
        for (int i = 0; i < num_nals; i++) {
            outFrameSize[i] = nals[i].i_payload;
            LOGE("encodeFrame data i %d , type %d,len %d,", i, nals[i].i_type, nals[i].i_payload);
            memcpy(outBytes + have_copy, nals[i].p_payload, nals[i].i_payload);
            have_copy += nals[i].i_payload;
        }
        LOGE("encodeFrame over ");
        return num_nals;
    }
    return -1;
}

int x264_release() {
    x264_encoder_close(encoder);
    x264_picture_clean(pic_in);
    x264_picture_clean(pic_out);
    free(pic_in);
    free(pic_out);
}

#endif //FFMPEGTEST_X264_TEST_H
