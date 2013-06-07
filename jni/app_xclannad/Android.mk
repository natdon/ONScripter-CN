LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := $(lastword $(subst /, ,$(LOCAL_PATH)))

APP_SUBDIR := $(firstword $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/xclannad* -type d)))

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(APP_SUBDIR) \
	$(LOCAL_PATH)/.. \
	$(LOCAL_PATH)/../sdl-1.2_jni/include \
	$(LOCAL_PATH)/../sdl_mixer/include \
	$(LOCAL_PATH)/../sdl_image/include \
	$(LOCAL_PATH)/../sdl_ttf/include \
	$(LOCAL_PATH)/../bz2/include \
	$(LOCAL_PATH)/../freetype/include \
	$(LOCAL_PATH)/../vorbis/include \
	$(LOCAL_PATH)/../ogg/include \
	$(LOCAL_PATH)/../mad/include \
	$(LOCAL_PATH)/../jpeg/include \
	$(LOCAL_PATH)/../png/include \
	$(LOCAL_PATH)/../tremor/include \
	$(LOCAL_PATH)/../video_jni 

LOCAL_CFLAGS := $(LOCAL_C_INCLUDES:%=-I%) \
	-DSDL_JAVA_PACKAGE_PATH=$(SDL_JAVA_PACKAGE_PATH) \
	-DMP3_MAD -DHAVE_CONFIG_H -DENABLE_PATCH -fsigned-char

LOCAL_CPPFLAGS += -fexceptions -frtti

#Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cc

OBJSUFFIX := .o
EXT_OBJS = 
include $(LOCAL_PATH)/$(APP_SUBDIR)/Makefile.xclannad
LOCAL_SRC_FILES := $(addprefix $(APP_SUBDIR)/,$(patsubst %.o, %.cc, $(XCLANNAD_OBJS)))

LOCAL_SHARED_LIBRARIES := sdl-1.2_jni sdl_mixer sdl_imagen sdl_ttfn mad bz2 lua video_jni freetype vorbis ogg mad jpeg png tremor

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz -lGLESv1_CM

LIBS_WITH_LONG_SYMBOLS := $(strip $(shell \
	for f in $(LOCAL_PATH)/../../libs/armeabi/*.so ; do \
		if echo $$f | grep "libapplication[.]so" > /dev/null ; then \
			continue ; \
		fi ; \
		if [ -e "$$f" ] ; then \
			if nm -g $$f | cut -c 12- | egrep '.{128}' > /dev/null ; then \
				echo $$f | grep -o 'lib[^/]*[.]so' ; \
			fi ; \
		fi ; \
	done \
) )

ifneq "$(LIBS_WITH_LONG_SYMBOLS)" ""
$(foreach F, $(LIBS_WITH_LONG_SYMBOLS), \
$(info Library $(F): abusing symbol names are: \
$(shell nm -g $(LOCAL_PATH)/../../libs/armeabi/$(F) | cut -c 12- | egrep '.{128}' ) ) \
$(info Library $(F) contains symbol names longer than 128 bytes, \
YOUR CODE WILL DEADLOCK WITHOUT ANY WARNING when you'll access such function - \
please make this library static to avoid problems. ) )
$(error Detected libraries with too long symbol names. Remove all files under project/libs/armeabi, make these libs static, and recompile)
endif

include $(BUILD_SHARED_LIBRARY)
