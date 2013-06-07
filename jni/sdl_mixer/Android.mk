LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl_mixer

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/.. $(LOCAL_PATH)/../sdl-1.2_jni/include \
	$(LOCAL_PATH)/../mad/include

LOCAL_CFLAGS := $(LOCAL_C_INCLUDES:%=-I%) \
	-DWAV_MUSIC -DMP3_MAD_MUSIC -DOGG_USE_TREMOR -DOGG_MUSIC

LOCAL_CPP_EXTENSION := .cpp

# Note this simple makefile var substitution, you can find even simpler examples in different Android projects
LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c)) $(addprefix timidity/, $(notdir $(wildcard $(LOCAL_PATH)/timidity/*.c)))

LOCAL_SHARED_LIBRARIES := sdl-1.2_jni tremor mad

LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

