/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// OpenGL ES 2.0 code

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "matrix.h"
#include "mesh.h"

#define USE_MESH 0 //switch to 1 to see mesh in action
#define  LOG_TAG    "libspinningcube"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}
static const char *gVertexShader = NULL;
static const char *gFragmentShader  = NULL;

GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint gProgram;
GLuint gvPositionHandle, gvBarPositionHandle, gvColorHandle;
GLuint gIntensityHandle;
GLuint gmvP;
int iXangle = 0, iYangle = 0, iZangle = 0;
float aRotate[16], aModelView[16], aPerspective[16], aMVP[16];
float uiWidth = 0.0f; float uiHeight = 0.0f;
static cMesh *gpMesh=NULL;

/* simple stats management */
typedef struct {
    double  renderTime;
    double  frameTime;
} FrameStats;

#define  MAX_FRAME_STATS  200
#define  MAX_PERIOD_MS    1500

typedef struct {
    double  firstTime;
    double  lastTime;
    double  frameTime;

    int         firstFrame;
    int         numFrames;
    FrameStats  frames[ MAX_FRAME_STATS ];
} Stats;

/* Return current time in milliseconds */
static double now_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000. + tv.tv_usec/1000.;
}

static void
stats_init( Stats*  s )
{
    s->lastTime = now_ms();
    s->firstTime = 0.;
    s->firstFrame = 0;
    s->numFrames  = 0;
}

static void
stats_startFrame( Stats*  s )
{
    s->frameTime = now_ms();
}

static void
stats_endFrame( Stats*  s )
{
    double now = now_ms();
    double renderTime = now - s->frameTime;
    double frameTime  = now - s->lastTime;
    int nn;

    if (now - s->firstTime >= MAX_PERIOD_MS) {
        if (s->numFrames > 0) {
            double minRender, maxRender, avgRender;
            double minFrame, maxFrame, avgFrame;
            int count;

            nn = s->firstFrame;
            minRender = maxRender = avgRender = s->frames[nn].renderTime;
            minFrame  = maxFrame  = avgFrame  = s->frames[nn].frameTime;
            for (count = s->numFrames; count > 0; count-- ) {
                nn += 1;
                if (nn >= MAX_FRAME_STATS)
                    nn -= MAX_FRAME_STATS;
                double render = s->frames[nn].renderTime;
                if (render < minRender) minRender = render;
                if (render > maxRender) maxRender = render;
                double frame = s->frames[nn].frameTime;
                if (frame < minFrame) minFrame = frame;
                if (frame > maxFrame) maxFrame = frame;
                avgRender += render;
                avgFrame  += frame;
            }
            avgRender /= s->numFrames;
            avgFrame  /= s->numFrames;

            LOGI("frame/s (avg,min,max) = (%.1f,%.1f,%.1f) "
                 "render time ms (avg,min,max) = (%.1f,%.1f,%.1f)\n",
                 1000./avgFrame, 1000./maxFrame, 1000./minFrame,
                 avgRender, minRender, maxRender);
        }
        s->numFrames  = 0;
        s->firstFrame = 0;
        s->firstTime  = now;
    }

    nn = s->firstFrame + s->numFrames;
    if (nn >= MAX_FRAME_STATS)
        nn -= MAX_FRAME_STATS;

    s->frames[nn].renderTime = renderTime;
    s->frames[nn].frameTime  = frameTime;

    if (s->numFrames < MAX_FRAME_STATS) {
        s->numFrames += 1;
    } else {
        s->firstFrame += 1;
        if (s->firstFrame >= MAX_FRAME_STATS)
            s->firstFrame -= MAX_FRAME_STATS;
    }

    s->lastTime = now;
}

/* 3D data. Vertex range -0.5..0.5 in all axes.
 * Z -0.5 is near, 0.5 is far. */
const float gCubeVertices[] =
{
    /* Front face. */
    /* Bottom left */
    -0.5,  0.5, -0.5,
    0.5, -0.5, -0.5,
    -0.5, -0.5, -0.5,
    /* Top right */
    -0.5,  0.5, -0.5,
    0.5,  0.5, -0.5,
    0.5, -0.5, -0.5,
    /* Left face */
    /* Bottom left */
    -0.5,  0.5,  0.5,
    -0.5, -0.5, -0.5,
    -0.5, -0.5,  0.5,
    /* Top right */
    -0.5,  0.5,  0.5,
    -0.5,  0.5, -0.5,
    -0.5, -0.5, -0.5,
    /* Top face */
    /* Bottom left */
    -0.5,  0.5,  0.5,
    0.5,  0.5, -0.5,
    -0.5,  0.5, -0.5,
    /* Top right */
    -0.5,  0.5,  0.5,
    0.5,  0.5,  0.5,
    0.5,  0.5, -0.5,
    /* Right face */
    /* Bottom left */
    0.5,  0.5, -0.5,
    0.5, -0.5,  0.5,
    0.5, -0.5, -0.5,
    /* Top right */
    0.5,  0.5, -0.5,
    0.5,  0.5,  0.5,
    0.5, -0.5,  0.5,
    /* Back face */
    /* Bottom left */
    0.5,  0.5,  0.5,
    -0.5, -0.5,  0.5,
    0.5, -0.5,  0.5,
    /* Top right */
    0.5,  0.5,  0.5,
    -0.5,  0.5,  0.5,
    -0.5, -0.5,  0.5,
    /* Bottom face */
    /* Bottom left */
    -0.5, -0.5, -0.5,
    0.5, -0.5,  0.5,
    -0.5, -0.5,  0.5,
    /* Top right */
    -0.5, -0.5, -0.5,
    0.5, -0.5, -0.5,
    0.5, -0.5,  0.5,
};

const float gCubeColors[] =
{
    /* Front face */
    /* Bottom left */
    1.0, 0.0, 0.0, /* red */
    0.0, 0.0, 1.0, /* blue */
    0.0, 1.0, 0.0, /* green */
    /* Top right */
    1.0, 0.0, 0.0, /* red */
    1.0, 1.0, 0.0, /* yellow */
    0.0, 0.0, 1.0, /* blue */
    /* Left face */
    /* Bottom left */
    1.0, 1.0, 1.0, /* white */
    0.0, 1.0, 0.0, /* green */
    0.0, 1.0, 1.0, /* cyan */
    /* Top right */
    1.0, 1.0, 1.0, /* white */
    1.0, 0.0, 0.0, /* red */
    0.0, 1.0, 0.0, /* green */
    /* Top face */
    /* Bottom left */
    1.0, 1.0, 1.0, /* white */
    1.0, 1.0, 0.0, /* yellow */
    1.0, 0.0, 0.0, /* red */
    /* Top right */
    1.0, 1.0, 1.0, /* white */
    0.0, 0.0, 0.0, /* black */
    1.0, 1.0, 0.0, /* yellow */
    /* Right face */
    /* Bottom left */
    1.0, 1.0, 0.0, /* yellow */
    1.0, 0.0, 1.0, /* magenta */
    0.0, 0.0, 1.0, /* blue */
    /* Top right */
    1.0, 1.0, 0.0, /* yellow */
    0.0, 0.0, 0.0, /* black */
    1.0, 0.0, 1.0, /* magenta */
    /* Back face */
    /* Bottom left */
    0.0, 0.0, 0.0, /* black */
    0.0, 1.0, 1.0, /* cyan */
    1.0, 0.0, 1.0, /* magenta */
    /* Top right */
    0.0, 0.0, 0.0, /* black */
    1.0, 1.0, 1.0, /* white */
    0.0, 1.0, 1.0, /* cyan */
    /* Bottom face */
    /* Bottom left */
    0.0, 1.0, 0.0, /* green */
    1.0, 0.0, 1.0, /* magenta */
    0.0, 1.0, 1.0, /* cyan */
    /* Top right */
    0.0, 1.0, 0.0, /* green */
    0.0, 0.0, 1.0, /* blue */
    1.0, 0.0, 1.0, /* magenta */
};

const float gBarVertices[] = 
{
    /* Front face. */
    /* Bottom left */
    -1.0,  1.0, -0.5,
    -0.8, -1.0, -0.5,
    -1.0, -1.0, -0.5,
    /* Top right */
    -1.0,  1.0, -0.5,
    -0.8,  1.0, -0.5,
    -0.8, -1.0, -0.5,
};

bool setupGraphics(int w, int h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics(%d, %d)", w, h);
    uiWidth = w; uiHeight = h;
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }
    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vPosition\") = %d\n", gvPositionHandle);

    gvColorHandle = glGetAttribLocation(gProgram, "vColor");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vColor\") = %d\n", gvColorHandle);

    //gIntensityHandle = glGetAttribLocation(gProgram, "intensity");
    //checkGlError("glGetAttribLocation");
    //LOGI("glGetAttribLocation(\"intensity\") = %d\n", gIntensityHandle);

    gmvP = glGetUniformLocation(gProgram, "mvp");
    checkGlError("glGetUniformLocation");
    LOGI("glGetUniformLocation(\"mvp\") = %d\n", gmvP);

    glViewport(0, 0, w, h);
    checkGlError("glViewport");
#if !USE_MESH
    glEnable(GL_CULL_FACE);
#endif
    glEnable(GL_DEPTH_TEST);

    //create a mesh
    gpMesh = new cMesh;
    gpMesh->buildPlane(3,2,3,2);
    gpMesh->computeVerticesNormals();
    return true;
}

void renderFrame() {
	static float grey;
	grey += 0.01f;
	if (grey > 1.0f) {
		grey = 0.0f;
	}
	glClearColor(grey, grey, grey, 1.0f);
	checkGlError("glClearColor");
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	checkGlError("glClear");

	glUseProgram(gProgram);
	checkGlError("glUseProgram");

	#if USE_MESH
	glVertexAttribPointer(gvPositionHandle, 4, GL_FLOAT, GL_FALSE, 0, gpMesh->m_pTriangleList);
	#else
	glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, gCubeVertices);
	#endif

	glEnableVertexAttribArray(gvPositionHandle);
	checkGlError("glEnableVertexAttribArray");

	#if USE_MESH
	glVertexAttribPointer(gvColorHandle, 3, GL_FLOAT, GL_FALSE, 0, gCubeColors); // <-hack! only works for 3X2 mesh
	checkGlError("glVertexAttribPointer");
	#else
	glVertexAttribPointer(gvColorHandle, 3, GL_FLOAT, GL_FALSE, 0, gCubeColors);
	checkGlError("glVertexAttribPointer");
	#endif

	glEnableVertexAttribArray(gvColorHandle);
	checkGlError("glEnableVertexAttribArray");

    
	//rotate - begin
	rotate_matrix(iXangle, 1.0, 0.0, 0.0, aModelView);
	rotate_matrix(iYangle, 0.0, 1.0, 0.0, aRotate);

	multiply_matrix(aRotate, aModelView, aModelView);

	rotate_matrix(iZangle, 0.0, 1.0, 0.0, aRotate);

	multiply_matrix(aRotate, aModelView, aModelView);

	/* Pull the camera back from the cube */
	aModelView[14] -= 2.5;

	perspective_matrix(45.0, (double)uiWidth/(double)uiHeight, 0.01, 100.0, aPerspective);
	multiply_matrix(aPerspective, aModelView, aMVP);

	glUniformMatrix4fv(gmvP, 1, GL_FALSE, aMVP);

	iXangle += 3;
	iYangle += 2;

	if(iXangle >= 360) iXangle -= 360;
	if(iXangle < 0) iXangle += 360;
	if(iYangle >= 360) iYangle -= 360;
	if(iYangle < 0) iYangle += 360;
	if(iZangle >= 360) iZangle -= 360;
	if(iZangle < 0) iZangle += 360;
	//rotate - end
#if USE_MESH
	glDrawArrays(GL_TRIANGLES, 0, gpMesh->m_numTriangles);
#else
	glDrawArrays(GL_TRIANGLES, 0, 36);
#endif
	checkGlError("glDrawArrays");
}
static Stats stats;
extern "C" {
    JNIEXPORT void JNICALL Java_com_android_spinningcube_SpinningCubeLib_init(JNIEnv * env, jobject obj,  jint width, jint height, jstring vShader, jstring fShader);
    JNIEXPORT void JNICALL Java_com_android_spinningcube_SpinningCubeLib_step(JNIEnv * env, jobject obj);
};

JNIEXPORT void JNICALL Java_com_android_spinningcube_SpinningCubeLib_init(JNIEnv * env, jobject obj,  jint width, jint height, jstring vShader, jstring fShader)
{
    if(!gVertexShader && !gFragmentShader) {
	gVertexShader = env->GetStringUTFChars(vShader, 0);
	gFragmentShader = env->GetStringUTFChars(fShader, 0);
    }
    setupGraphics(width, height);
    stats_init(&stats);

}

JNIEXPORT void JNICALL Java_com_android_spinningcube_SpinningCubeLib_step(JNIEnv * env, jobject obj)
{
    stats_startFrame(&stats);
    renderFrame();
    stats_endFrame(&stats);
}
