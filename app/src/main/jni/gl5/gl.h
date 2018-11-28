#ifndef FFMPEGTEST_EGL_H
#define FFMPEGTEST_EGL_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android/log.h>
#include <jni.h>
#include <libavutil/frame.h>

#define LOGV(FORMAT, ...) __android_log_print(ANDROID_LOG_ERROR,"LC XXX",FORMAT,##__VA_ARGS__);

typedef struct GlobalContexts {
    EGLDisplay eglDisplay;
    EGLSurface eglSurface;
    EGLContext eglContext;
    EGLint eglFormat;

    int width;
    int height;

} GlobalContext;

void display();

//2绑定window
void setSurface(JNIEnv *env, jobject obj, jobject surface);

//3设置windows
int32_t setBuffersGeometry(int32_t width, int32_t height);

#endif
