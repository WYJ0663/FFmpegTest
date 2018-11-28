#ifndef CM_MATRIX_H
#define CM_MATRIX_H
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <GLES2/gl2.h>

#define PI 3.14159265f

#define LOG_TAG_MATRIX "matrix"
#define LOGI_MATRIX(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG_MATRIX,__VA_ARGS__)
#define LOGW_MATRIX(...) __android_log_print(ANDROID_LOG_WARN,LOG_TAG_MATRIX,__VA_ARGS__)


void
printArray(char* name, float* rm);

//���֮�䳤��
float
length (float x, float y, float z);

float *test_loadOrtho();

float* IJK_GLES2_loadOrtho( GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far);

//������ת����
float*
getRotateM (float* m, int rmOffset, float a, float x, float y, float z);

//ƽ�ƾ���
void
translateM (float* m, int mOffset, float x, float y, float z);

//����ͷ����
float*
setLookAtM (float* rm, int rmOffset, float eyeX, float eyeY, float eyeZ,
	    float centerX, float centerY, float centerZ, float upX, float upY,
	    float upZ);

/**
 * 4x4������
 * param left : ���Ӿ��,�K�������ŽY��
 */
void
matrixMM4 (float* left, float* right);

/**
 * ͸�Ӿ���
 *
 */
float*
frustumM (float* m, int offset, float left, float right, float bottom,
	  float top, float near, float far);

#endif
