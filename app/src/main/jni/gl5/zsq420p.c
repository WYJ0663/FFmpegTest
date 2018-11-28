//
// Created by yijunwu on 2018/10/26.
//
#include <GLES2/gl2.h>
#include <malloc.h>
#include <stdbool.h>
#include <android/native_window_jni.h>
#include <libavutil/frame.h>

#include "zsq420p.h"
#include "gl.h"

#define UNIT 1
#define TEXTURE_COOR_UNIT 1

//渲染顶点坐标数据
const float dataVertex[] =
        {
                -1 * UNIT, 1 * UNIT,
                -1 * UNIT, -1 * UNIT,
                1 * UNIT, 1 * UNIT,
                1 * UNIT, -1 * UNIT
        };
//渲染纹理坐标数据
const float dataTexCoor[] =
        {
                0 * TEXTURE_COOR_UNIT, 0 * TEXTURE_COOR_UNIT,
                0 * TEXTURE_COOR_UNIT, 1 * TEXTURE_COOR_UNIT,
                1 * TEXTURE_COOR_UNIT, 0 * TEXTURE_COOR_UNIT,
                1 * TEXTURE_COOR_UNIT, 1 * TEXTURE_COOR_UNIT
        };



///////////////////// 米2S、华为 等//////////////////////////
GLfloat squareVertices_zero[] = {
        0.0f,   1.0f,
        1.0f,   1.0f,
        0.0f,   0.0f,
        1.0f,   0.0f,
};
GLfloat coordVertices_zero[] = {
        -1.0f,  -1.0f,
        1.0f,  -1.0f,
        -1.0f,   1.0f,
        1.0f,   1.0f,
};

////////////////////// 红米、台电pad、kindle pad、SS_SCH-1939D等//////////////////////////
GLfloat squareVertices_one[] = {
        -1.0f,  -1.0f,
        1.0f,  -1.0f,
        -1.0f,   1.0f,
        1.0f,   1.0f,
};

GLfloat coordVertices_one[] = {
        0.0f,   1.0f,
        1.0f,   1.0f,
        0.0f,   0.0f,
        1.0f,   0.0f,
};

//顶点着色器脚本代码
const char *codeVertexShader = \
"attribute vec3 aPosition;							\n" \
"uniform mat4 uMVPMatrix;	 						\n" \
"attribute vec2 aTexCoor; 							\n" \
"varying vec2 vTexCoor;		 						\n" \
"void main() 										\n" \
"{ 													\n" \
"	gl_Position = uMVPMatrix*vec4(aPosition, 1); 	\n" \
" 	vTexCoor = aTexCoor;							\n" \
"} 													\n" \
;

//-------------MATH---------------
const char *codeFragShader = \
"precision mediump float;											\n" \
"uniform sampler2D yTexture; 										\n" \
"uniform sampler2D uTexture; 										\n" \
"uniform sampler2D vTexture; 										\n" \
"varying vec2 vTexCoor;												\n" \
"void main()														\n" \
"{																	\n" \
"	float y = texture2D(yTexture, vTexCoor).r;						\n" \
"	float u = texture2D(uTexture, vTexCoor).r;											\n" \
"	float v = texture2D(vTexture, vTexCoor).r;													\n" \
"	vec3 yuv = vec3(y, u, v);												\n" \
"	vec3 offset = vec3(16.0 / 255.0, 128.0 / 255.0, 128.0 / 255.0);								\n" \
"	mat3 mtr = mat3(1.0, 1.0, 1.0, -0.001, -0.3441, 1.772, 1.402, -0.7141, 0.001);						\n" \
"	vec4 curColor = vec4(mtr * (yuv - offset), 1);												\n" \
"	gl_FragColor = curColor;	"
"												\n" \
"}																	\n" \
;


Instance *instance;


void init(int pWidth, int pHeight) {
    LOGE("init()");
    //创建一个引用
    //创建一个引用
    instance = (Instance *) malloc(sizeof(Instance));
    memset(instance, 0, sizeof(Instance));

    GLuint shaders[2] = {0};
    //创建顶点shader和片元shader
    shaders[0] = initShader(codeVertexShader, GL_VERTEX_SHADER);
    shaders[1] = initShader(codeFragShader, GL_FRAGMENT_SHADER);
    //编译链接shader
    instance->pProgram = initProgram(shaders, 2);

    //获取mvp矩阵的索引
    instance->maMVPMatrixHandle = glGetUniformLocation(instance->pProgram, "uMVPMatrix");
    //获取顶点坐标索引
    instance->maPositionHandle = glGetAttribLocation(instance->pProgram, "aPosition");
    //获取纹理坐标索引
    instance->maTexCoorHandle = glGetAttribLocation(instance->pProgram, "aTexCoor");
    //获取采样器索引
    instance->myTextureHandle = glGetUniformLocation(instance->pProgram, "yTexture");
    instance->muTextureHandle = glGetUniformLocation(instance->pProgram, "uTexture");
    instance->mvTextureHandle = glGetUniformLocation(instance->pProgram, "vTexture");

    //获取对象名称 这里分别返回1个用于纹理对象的名称，后面为对应纹理赋值时将以这个名称作为索引
    glGenTextures(1, &instance->yTexture);
    glGenTextures(1, &instance->uTexture);
    glGenTextures(1, &instance->vTexture);

    LOGE("init() yT = %d, uT = %d, vT = %d.", instance->yTexture, instance->uTexture, instance->vTexture);
    LOGE("%s %d error = %d", __FILE__, __LINE__, glGetError());

    //为yuv数据分配存储空间
    instance->yBufferSize = sizeof(char) * pWidth * pHeight;
    instance->uBufferSize = sizeof(char) * pWidth / 2 * pHeight / 2;
    instance->vBufferSize = sizeof(char) * pWidth / 2 * pHeight / 2;
    instance->yBuffer = (char *) malloc(instance->yBufferSize);
    instance->uBuffer = (char *) malloc(instance->uBufferSize);
    instance->vBuffer = (char *) malloc(instance->vBufferSize);
    memset(instance->yBuffer, 0, instance->yBufferSize);
    memset(instance->uBuffer, 0, instance->uBufferSize);
    memset(instance->vBuffer, 0, instance->vBufferSize);
    //指定图像大小
    instance->pHeight = pHeight;
    instance->pWidth = pWidth;
    LOGE("width = %d, height = %d", instance->pWidth, instance->pHeight);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

//  glEnable(GL_DEPTH_TEST);
    LOGE("%s %d error = %d", __FILE__, __LINE__, glGetError());
}

void bindTexture(GLenum texture_n, GLuint texture_id, GLsizei width, GLsizei height, const void *buffer) {
    LOGI_EU("texture_n = %x, texture_id = %d, width = %d, height = %d", texture_n, texture_id, width, height);
    //处理纹理
    //		2.绑定纹理
    glActiveTexture(texture_n);//eg:GL_TEXTURE0
    //		1.1绑定纹理id
    glBindTexture(GL_TEXTURE_2D, texture_id);
    //		2.3设置采样模式
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //		1.2输入纹理数据
//    glTexImage2D(GL_TEXTURE_2D,
//                 0,//GLint level
//                 GL_LUMINANCE,//GLint internalformat
//                 width,//GLsizei widthglTexImage2DglTexImage2D
//                 height,// GLsizei height,
//                 0,//GLint border,
//                 GL_LUMINANCE,//GLenum format,
//                 GL_UNSIGNED_BYTE,//GLenum type,
//                 buffer//const void * pixels
//    );

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_LUMINANCE,
                 width,
                 height,
                 0,
                 GL_LUMINANCE,
                 GL_UNSIGNED_BYTE,
                 buffer);


    LOGE("%s %d error = %d", __FILE__, __LINE__, glGetError());
};

void
drawFrame(void *ins) {
    LOGI_EU("%s", __FUNCTION__);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDisable(GL_DEPTH_TEST);
    LOGE("%s %d error = %d", __FILE__, __LINE__, glGetError());
    Instance *instance = (Instance *) ins;
    if (instance == 0) {
        LOGW_EU("%s Program is NULL return!", __FUNCTION__);
        return;
    }
    LOGE("%s %d error = %d", __FILE__, __LINE__, glGetError());
    //使用编译好的program
    glUseProgram(instance->pProgram);
//    float *maMVPMatrix =  IJK_GLES2_loadOrtho( -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    float *maMVPMatrix = test_loadOrtho();
    //图像旋转270度
//    float *maMVPMatrix = getRotateM(NULL, 0, 270, 0, 0, 1);
    //float * maMVPMatrix = getRotateM(NULL, 0, 0, 0, 0, 1);
    //传入mvp矩阵
    glUniformMatrix4fv(instance->maMVPMatrixHandle, 1, GL_FALSE, maMVPMatrix);
//
    free(maMVPMatrix);


    //传入顶点坐标
    LOGE("%s %d error = %d", __FILE__, __LINE__, glGetError());
    glVertexAttribPointer(instance->maPositionHandle,
                          2,//GLint size X Y Z
                          GL_FLOAT,//GLenum type
                          GL_FALSE,//GLboolean normalized
                          0,//GLsizei stride  dataVertex中三个数据一组
                          dataVertex//const GLvoid * ptr
    );
    LOGE("%s %d error = %d", __FILE__, __LINE__, glGetError());
    //传入纹理坐标
    glVertexAttribPointer(instance->maTexCoorHandle,
                          2,//S T
                          GL_FLOAT,//GLenum type
                          GL_FALSE,//GLboolean normalized
                          0,//GLsizei stride   dataTexCoor中两个数据一组
                          dataTexCoor//const GLvoid * ptr
    );
    LOGE("%s %d error = %d", __FILE__, __LINE__, glGetError());
    //绑定纹理
    bindTexture(GL_TEXTURE0, instance->yTexture, instance->pWidth, instance->pHeight, instance->yBuffer);
    bindTexture(GL_TEXTURE1, instance->uTexture, instance->pWidth / 2, instance->pHeight / 2, instance->uBuffer);
    bindTexture(GL_TEXTURE2, instance->vTexture, instance->pWidth / 2, instance->pHeight / 2, instance->vBuffer);

    //片元中uniform 2维均匀变量赋值
    glUniform1i(instance->myTextureHandle, 0); //对应纹理第1层
    glUniform1i(instance->muTextureHandle, 1); //对应纹理第2层
    glUniform1i(instance->mvTextureHandle, 2); //对应纹理第3层

    //enable之后这些引用才能在shader中生效
    glEnableVertexAttribArray(instance->maPositionHandle);
    glEnableVertexAttribArray(instance->maTexCoorHandle);

    //绘制 从顶点0开始绘制，总共四个顶点，组成两个三角形，两个三角形拼接成一个矩形纹理，也就是我们的画面
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    LOGE("%s %d error = %d", __FILE__, __LINE__, glGetError());
}

//渲染数据
void drawFrame2(uint8_t *srcp) {
    //将yuv数据分别copy到对应的buffer中

    memcpy(instance->yBuffer, srcp, instance->yBufferSize);
    memcpy(instance->uBuffer, srcp + instance->yBufferSize, instance->uBufferSize);
    memcpy(instance->vBuffer, srcp + instance->yBufferSize + instance->uBufferSize, instance->vBufferSize);

    //opengl绘制
    drawFrame(instance);

    //交换display中显示图像缓存的地址和后台图像缓存的地址，将当前计算出的图像缓存显示
    display();

}

//渲染数据
void drawFrame3(uint8_t *y, uint8_t *u, uint8_t *v) {
    //将yuv数据分别copy到对应的buffer中

    memcpy(instance->yBuffer, y, instance->yBufferSize);
    memcpy(instance->uBuffer, u, instance->uBufferSize);
    memcpy(instance->vBuffer, v, instance->vBufferSize);

    //opengl绘制
    drawFrame(instance);

    //交换display中显示图像缓存的地址和后台图像缓存的地址，将当前计算出的图像缓存显示
    display();

}


////////////////////////////

struct GlobalContexts global_context;

ANativeWindow *mANativeWindow;

// format not used now.
int32_t setBuffersGeometry(int32_t width, int32_t height) {
    //int32_t format = WINDOW_FORMAT_RGB_565;
    global_context.width = width;
    global_context.height = height;

    if (NULL == mANativeWindow) {
        LOGV("mANativeWindow is NULL.");
        return -1;
    }

    return ANativeWindow_setBuffersGeometry(mANativeWindow, width, height,
                                            global_context.eglFormat);
}

int eglOpen() {

    EGLDisplay eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        LOGV("eglGetDisplay failure.");
        return -1;
    }
    global_context.eglDisplay = eglDisplay;
    LOGV(" eglGetDisplay ok");

    EGLint majorVersion;
    EGLint minorVersion;
    EGLBoolean success = eglInitialize(eglDisplay, &majorVersion,
                                       &minorVersion);
    if (!success) {
        LOGV(" eglInitialize failure.");
        return -1;
    }
    LOGV(" eglInitialize ok");

    GLint numConfigs;
    EGLConfig config;
    static const EGLint CONFIG_ATTRIBS[] = {EGL_BUFFER_SIZE, EGL_DONT_CARE,
                                            EGL_RED_SIZE, 5,
                                            EGL_GREEN_SIZE, 6,
                                            EGL_BLUE_SIZE, 5,
                                            EGL_DEPTH_SIZE, 16,
                                            EGL_ALPHA_SIZE, EGL_DONT_CARE,
                                            EGL_STENCIL_SIZE, EGL_DONT_CARE,
                                            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                                            EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE // the end
    };

    static const EGLint configAttribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };

    success = eglChooseConfig(eglDisplay, configAttribs, &config, 1,
                              &numConfigs);
    if (!success) {
        LOGV(" eglChooseConfig failure.");
        return -1;
    }
    LOGV(" eglChooseConfig ok");

    const EGLint attribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    EGLContext elgContext = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT,
                                             attribs);
    if (elgContext == EGL_NO_CONTEXT) {
        LOGV(" eglCreateContext failure, error is %d", eglGetError());
        return -1;
    }
    global_context.eglContext = elgContext;
    LOGV(" eglCreateContext ok");

    EGLint eglFormat;
    success = eglGetConfigAttrib(eglDisplay, config, EGL_NATIVE_VISUAL_ID,
                                 &eglFormat);
    if (!success) {
        LOGV(" eglGetConfigAttrib failure.");
        return -1;
    }
    global_context.eglFormat = eglFormat;
    LOGV(" eglGetConfigAttrib ok");

    EGLSurface eglSurface = eglCreateWindowSurface(eglDisplay, config,
                                                   mANativeWindow, 0);
    if (NULL == eglSurface) {
        LOGV(" eglCreateWindowSurface failure.");
        return -1;
    }
    global_context.eglSurface = eglSurface;

    LOGV(" eglCreateWindowSurface ok");

    //关联屏幕,后台回来会断开
    EGLBoolean is = eglMakeCurrent(eglDisplay, eglSurface, eglSurface, elgContext);
    if (is) {
        LOGV(" eglMakeCurrent ok");
    }

    return 0;
}

int eglClose() {
    EGLBoolean success = eglDestroySurface(global_context.eglDisplay, global_context.eglSurface);
    if (!success) {
        LOGV("eglDestroySurface failure.");
    }

    success = eglDestroyContext(global_context.eglDisplay, global_context.eglContext);
    if (!success) {
        LOGV("eglDestroySurface failure.");
    }

    success = eglTerminate(global_context.eglDisplay);
    if (!success) {
        LOGV("eglDestroySurface failure.");
    }

    global_context.eglSurface = NULL;
    global_context.eglContext = NULL;
    global_context.eglDisplay = NULL;

    return 0;
}


void setSurface(JNIEnv *env, jobject obj, jobject surface) {

    // obtain a native window from a Java surface
    mANativeWindow = ANativeWindow_fromSurface(env, surface);

    LOGV("mANativeWindow ok");
    if ((global_context.eglSurface != NULL)
        || (global_context.eglContext != NULL)
        || (global_context.eglDisplay != NULL)) {
        eglClose();
    }
    eglOpen();
}


void display() {
    EGLBoolean res = eglSwapBuffers(global_context.eglDisplay, global_context.eglSurface);

    if (res == EGL_FALSE) {
        LOGV("eglSwapBuffers Error %d", eglGetError());
    } else {
        LOGV("eglSwapBuffers Ok");
    }
}