LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := video_jni

ifndef SDL_JAVA_PACKAGE_PATH
$(error Please define SDL_JAVA_PACKAGE_PATH to the path of your Java package with dots replaced with underscores, for example "com_example_SanAngeles")
endif
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CFLAGS := -DSDL_JAVA_PACKAGE_PATH=$(SDL_JAVA_PACKAGE_PATH) -DSDL_ANDROID_PACKAGE_NAME=\"$(SDL_ANDROID_PACKAGE_NAME)\"

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := video_jni.cpp


LOCAL_LDLIBS := -llog 

include $(BUILD_SHARED_LIBRARY)
