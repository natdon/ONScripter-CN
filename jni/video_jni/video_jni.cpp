#ifdef ANDROID
extern "C"
{
#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "include/videojni.h"
#include "include/font.h"

static JavaVM *jniVM = NULL;
static jobject JavaONScripter = NULL;
static jmethodID JavaPlayVideo = NULL;

int font_x;
int font_y;
int font_px;
int font_py;

int font_color1;
int font_color2;
int font_color3;

bool UseColor = false;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    jniVM = vm;
    return JNI_VERSION_1_2;
};

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
    jniVM = vm;
};

#ifndef SDL_JAVA_PACKAGE_PATH
#error You have to define SDL_JAVA_PACKAGE_PATH to your package path with dots replaced with underscores, for example "com_example_SanAngeles"
#endif
#define JAVA_EXPORT_NAME2(name,package) Java_##package##_##name
#define JAVA_EXPORT_NAME1(name,package) JAVA_EXPORT_NAME2(name,package)
#define JAVA_EXPORT_NAME(name) JAVA_EXPORT_NAME1(name,SDL_JAVA_PACKAGE_PATH)

JNIEXPORT jint JNICALL JAVA_EXPORT_NAME(ONScripter_nativeInitJavaCallbacks) (JNIEnv * jniEnv, jobject thiz)
{
	__android_log_print(ANDROID_LOG_ERROR, "ONS", "sd");
    JavaONScripter = jniEnv->NewGlobalRef(thiz);
    jclass JavaONScripterClass = jniEnv->GetObjectClass(JavaONScripter);
    JavaPlayVideo = jniEnv->GetMethodID(JavaONScripterClass, "playVideo", "([C)V");
	__android_log_print(ANDROID_LOG_ERROR, "ONS", "sd2");
}

JNIEXPORT jint JNICALL 
JAVA_EXPORT_NAME(ONScripter_nativeFontSize) ( JNIEnv*  env, jobject thiz,jint f_x,jint f_y,jint f_px, jint f_py )
{
	font_x = f_x;
	font_y = f_y;
	font_px = f_px;
	font_py = f_py;
	return 0;
}

JNIEXPORT jint JNICALL 
JAVA_EXPORT_NAME(ONScripter_nativeFontColor) ( JNIEnv*  env, jobject thiz, jboolean usecolor,jint fcolor1,jint fcolor2,jint fcolor3)
{
	UseColor = usecolor;
	font_color1 = fcolor1;
	font_color2 = fcolor2;
	font_color3 = fcolor3;
	return 0;
}

}

void playVideoAndroid(const char *filename)
{__android_log_print(ANDROID_LOG_ERROR, "ONS", "sd3");
    JNIEnv * jniEnv = NULL;
    jniVM->AttachCurrentThread(&jniEnv, NULL);

    if (!jniEnv){
        __android_log_print(ANDROID_LOG_ERROR, "ONS", "ONScripter::playVideoAndroid: Java VM AttachCurrentThread() failed");
        return;
    }

    jchar *jc = new jchar[strlen(filename)];
    for (int i=0 ; i<strlen(filename) ; i++)
        jc[i] = filename[i];
    jcharArray jca = jniEnv->NewCharArray(strlen(filename));
    jniEnv->SetCharArrayRegion(jca, 0, strlen(filename), jc);
    jniEnv->CallIntMethod( JavaONScripter, JavaPlayVideo, jca );
    delete[] jc;
}

int fontx()
{
	return font_x;
}

int fonty()
{
	return font_y;
}

int fontpx()
{
	return font_px;
}

int fontpy()
{
	return font_py;
}

bool RetUseColor()
{
	return UseColor;
}

int Retfc1()
{
	return font_color1;
}

int Retfc2()
{
	return font_color2;
}

int Retfc3()
{
	return font_color3;
}
#endif
