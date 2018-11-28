#include <android/native_window_jni.h>

//编码
#include "libavcodec/avcodec.h"
//封装格式处理
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
//像素处理
#include "libswscale/swscale.h"

#include <unistd.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <libavutil/imgutils.h>
#include "Log.h"
#include "ff_packet_queue.h"
#include "gl5/gl.h"
#include "gl5/zsq420p.h"


unsigned char *change(AVFrame *pFrame_, int height) {
    int newSize = (pFrame_->linesize[0] + pFrame_->linesize[1] + pFrame_->linesize[2]) * height;

    //申请内存
    unsigned char *buf = av_malloc(newSize);

    //写入数据
    int a = 0, i;
    for (i = 0; i < height; i++) {
        memcpy(buf + a, pFrame_->data[0] + i * pFrame_->linesize[0], pFrame_->linesize[0]);
        a += pFrame_->linesize[0];
    }
    for (i = 0; i < height / 2; i++) {
        memcpy(buf + a, pFrame_->data[1] + i * pFrame_->linesize[1], pFrame_->linesize[1]);
        a += pFrame_->linesize[1];
    }
    for (i = 0; i < height / 2; i++) {
        memcpy(buf + a, pFrame_->data[2] + i * pFrame_->linesize[2], pFrame_->linesize[2]);
        a += pFrame_->linesize[2];
    }

    //===============
    //到这里，buf里面已经是yuv420p的数据了，可以对它做任何的处理拉！
    //===============
    return buf;
}

JNIEXPORT void JNICALL
Java_com_ffmpeg_Play__1play(JNIEnv *env, jobject instance, jstring inputPath_) {


    const char *url = (*env)->GetStringUTFChars(env, inputPath_, 0);

    AVFormatContext *pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
        LOGE("Couldn't open file:%s\n", url);
        return; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        LOGE("Couldn't find stream information.");
        return;
    }

    int videoIndex;
    AVCodecContext *videoCodec;
    int audioIndex;
    AVCodecContext *audioCodec;

    for (int i = 0; i < pFormatCtx->nb_streams; ++i) {
        //获取解码器
        AVCodec *avCodec = avcodec_find_decoder(pFormatCtx->streams[i]->codecpar->codec_id);
        AVCodecContext *codecContext = avcodec_alloc_context3(avCodec);
        avcodec_parameters_to_context(codecContext, pFormatCtx->streams[i]->codecpar);

//        AVCodecContext *avCodecContext = pFormatCtx->streams[i]->codec;
//        AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);
//
//        //copy一个解码器，
//        AVCodecContext *codecContext = avcodec_alloc_context3(avCodec);
//        avcodec_copy_context(codecContext, avCodecContext);
        if (avcodec_open2(codecContext, avCodec, NULL) < 0) {
            LOGE("打开失败")
            continue;
        }

        //如果是视频流
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoIndex = i;
            videoCodec = codecContext;
            setBuffersGeometry(videoCodec->width, videoCodec->height);
            init(videoCodec->coded_width, videoCodec->height);
            switch (videoCodec->pix_fmt) {
                case AV_PIX_FMT_YUV420P:
                    LOGE("get_format %s ", "AV_PIX_FMT_YUV420P");
                    break;
                case AV_PIX_FMT_YUVJ420P:
                    LOGE("get_format %s ", "AV_PIX_FMT_YUVJ420P");
                    break;
            }
            LOGE("get_format %d ", videoCodec->pix_fmt);
        }//如果是音频流
        else if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioIndex = i;
            audioCodec = codecContext;
        }
    }

    enum AVPixelFormat format = AV_PIX_FMT_RGB24;//AV_PIX_FMT_RGB565LE
    AVFrame *rgb_frame = av_frame_alloc();
    //缓存区
    uint8_t *out_buffer = (uint8_t *) av_mallocz((size_t) av_image_get_buffer_size(format,
                                                                                   videoCodec->width,
                                                                                   videoCodec->height, 1));
    //与缓存区相关联，设置rgb_frame缓存区
    av_image_fill_arrays(rgb_frame->data, rgb_frame->linesize, out_buffer, format, videoCodec->width,
                         videoCodec->height, 1);

    struct SwsContext *swsContext = sws_getContext(videoCodec->width, videoCodec->height,
                                                   videoCodec->pix_fmt,
                                                   videoCodec->width, videoCodec->height, format,
                                                   SWS_BICUBIC, NULL, NULL, NULL);

//    std::vector<AVPacket *> queue;//队列
    Queue *queue2 = createQueue();

    AVPacket *avPacket = av_packet_alloc();
    AVFrame *avFrame = av_frame_alloc();
    //跳转到某一个特定的帧上面播放
    int ret;
    int i = 0;
    while (1) {

        LOGE("第几帧=%d", i++);
        ret = av_read_frame(pFormatCtx, avPacket);

//        queue.push_back(packet);

        if (ret == 0) {
            AVPacket *packet = av_packet_alloc();
            av_packet_ref(packet, avPacket);
            enQueue(queue2, packet);

            if (avPacket->stream_index == videoIndex) {
                avcodec_send_packet(videoCodec, avPacket);
                if (avcodec_receive_frame(videoCodec, avFrame) == 0) {
                    // Convert the image into YUV format that SDL uses
//                    sws_scale(swsContext, (const uint8_t *const *) avFrame->data, avFrame->linesize, 0,
//                              avFrame->height, rgb_frame->data,
//                              rgb_frame->linesize);
                    LOGE("解码第几帧=%d", i++);
//                    drawFrame2(change(avFrame, videoCodec->height));
                    drawFrame3(avFrame);
//                    call_video_play(rgb_frame);
                }
            } else if (avPacket->stream_index == audioIndex) {
//                if (avcodec_receive_frame(audioCodec, avFrame) != 0) {
//
//                }
            }
        } else if (ret == AVERROR_EOF) {
            break;
        } else {
            break;
        }
        av_packet_unref(avPacket);
        av_frame_unref(avFrame);

    }
    cleanQueue(queue2);
    freeQueue(queue2);

    av_packet_unref(avPacket);
    av_packet_free(&avPacket);
    av_frame_unref(avFrame);
    av_frame_free(&avFrame);
    avformat_close_input(&pFormatCtx);
    avcodec_free_context(&videoCodec);
    avcodec_free_context(&audioCodec);
    LOGE("回收资源");
}


JNIEXPORT void JNICALL
Java_com_ffmpeg_Play__1display(JNIEnv *env, jobject instance, jobject surface) {

    setSurface(env, instance, surface);
}


JNIEXPORT void JNICALL
Java_com_ffmpeg_Play__1stop(JNIEnv *env, jobject instance) {
    LOGE("结束");
}


JNIEXPORT void JNICALL
Java_com_ffmpeg_Play__1pause(JNIEnv *env, jobject instance) {
    LOGE("结束");
}


JNIEXPORT void JNICALL
Java_com_ffmpeg_Play__1seekTo(JNIEnv *env, jobject instance, jint msec) {

}


JNIEXPORT void JNICALL
Java_com_ffmpeg_Play__1silence(JNIEnv *env, jobject instance) {
}


JNIEXPORT void JNICALL
Java_com_ffmpeg_Play__1rate(JNIEnv *env, jobject instance, jfloat rate) {
    LOGE("play music->rate 2 =%f", rate)
}


JNIEXPORT void JNICALL
Java_com_ffmpeg_Play__1cut(JNIEnv *env, jobject instance) {
}


JNIEXPORT jstring JNICALL
Java_com_ffmpeg_Play__1configuration(JNIEnv *env, jobject instance) {

    // TODO
    char info[10000] = {0};
//    av_register_all();

    sprintf(info, "%s\n", avcodec_configuration());

    return (*env)->NewStringUTF(env, info);
}

