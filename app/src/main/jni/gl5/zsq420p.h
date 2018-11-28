//
// Created by yijunwu on 2018/10/26.
//

#ifndef FFMPEGTEST_ZSQ420P_H
#define FFMPEGTEST_ZSQ420P_H


#include <GLES2/gl2.h>
#include <malloc.h>
#include <memory.h>
#include <EGL/egl.h>
#include "../Log.h"
#include "opengles/esUtil.h"



typedef struct Instance {
    //顶点着色器位置数据引用
    unsigned int maPositionHandle;
    //顶点着色器纹理坐标引用
    unsigned int maTexCoorHandle;
    //着色器程序引用
    unsigned int pProgram;
    //顶点着色器最终变化矩阵引用
    unsigned int maMVPMatrixHandle;
    //片元着色器采样器引用
    unsigned int myTextureHandle;
    unsigned int muTextureHandle;
    unsigned int mvTextureHandle;
    //纹理数据
    unsigned int yTexture;
    unsigned int uTexture;
    unsigned int vTexture;
    //着色器渲染宽高
    unsigned int pWidth;
    unsigned int pHeight;
    //屏幕的宽高
    unsigned int vWidth;
    unsigned int vHeight;
    //yuv数据
    signed char *yBuffer;
    unsigned long yBufferSize;
    signed char *uBuffer;
    unsigned long uBufferSize;
    signed char *vBuffer;
    unsigned long vBufferSize;
} Instance;

void init(int pWidth, int pHeight);

void drawFrame2(uint8_t *srcp);

void drawFrame3(uint8_t *y, uint8_t *u,uint8_t *v);

#endif //FFMPEGTEST_ZSQ420P_H
