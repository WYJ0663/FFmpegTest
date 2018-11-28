/***此类用于生成一些OpenGL ES 2.0 的对象****/
#ifndef _CM_ES_UTIL_
#define _CM_ES_UTIL_
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include "matrix.h"

#define LOG_TAG "yijun2"
#define LOGI_EU(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define LOGW_EU(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)


/**
 * 初始化着色器
 */
GLint
initShader(const char * strShaderCode, unsigned int shaderType);

/**
 * 检查着色器初始化状态
 */
GLint
checkInitShader(GLint pShader);

/**
 * 初始化渲染程序
 */
GLint
initProgram(GLuint* shaderArray, GLint size);

/**
 * 检查程序链接状态
 */
GLint
checkLinkProgram(GLint pProgram);


#endif
