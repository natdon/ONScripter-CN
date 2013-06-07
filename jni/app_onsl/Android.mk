

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := $(lastword $(subst /, ,$(LOCAL_PATH)))

APP_SUBDIR := $(firstword $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/onsl* -type d)))

LOCAL_CFLAGS := $(foreach D, $(APP_SUBDIR), -I$(LOCAL_PATH)/$(D)) \
	-I$(LOCAL_PATH)/.. \
	-I$(LOCAL_PATH)/../sdl-1.2_jni/include \
	-I$(LOCAL_PATH)/../sdl_image/include \
	-I$(LOCAL_PATH)/../bz2/include \
	-I$(LOCAL_PATH)/../freetype/include \
	-I$(LOCAL_PATH)/../vorbis/include \
	-I$(LOCAL_PATH)/../ogg/include \
	-I$(LOCAL_PATH)/../mikmod/include \
	-I$(LOCAL_PATH)/../flac/include \
	-I$(LOCAL_PATH)/../timidity/include \
	-I$(LOCAL_PATH)/../openal/include \
	-I$(LOCAL_PATH)/../jpeg/include \
	-I$(LOCAL_PATH)/../png/include \
	-I$(LOCAL_PATH)/../tremor/include \
	-I$(LOCAL_PATH)/../mpg123/libmpg123 \
	-I$(LOCAL_PATH)/../video_jni 
	-DSDL_JAVA_PACKAGE_PATH=$(SDL_JAVA_PACKAGE_PATH) \
	-DUNICODE -DNONS_SVN -DLINUX -DANDROID -DNONS_SYS_UNIX -DUSE_THREAD_MANAGER -DONLY_NEAREST_NEIGHBOR
	
	
LOCAL_CPPFLAGS += -fpermissive -fno-exceptions

#Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cpp


LOCAL_C_INCLUDES+= \
onsl/libFLAC++ \
onsl/libFLAC++/share \
onsl/libtimidity \
onsl/Plugin \
onsl/tinyxml \


LOCAL_SRC_FILES := \
onsl/Archive.cpp \
onsl/Audio.cpp \
onsl/AudioFormats.cpp \
onsl/ErrorCodes.cpp \
onsl/ExpressionParser.tab.cpp \
onsl/FileLog.cpp \
onsl/Functions.cpp \
onsl/GFX.cpp \
onsl/GUI.cpp \
onsl/Image.cpp \
onsl/INIfile.cpp \
onsl/INIParser.tab.cpp \
onsl/IOFunctions.cpp \
onsl/libFLAC++/metadata.cpp \
onsl/libFLAC++/stream_decoder.cpp \
onsl/libFLAC++/stream_encoder.cpp \
onsl/libtimidity/common.c \
onsl/libtimidity/instrum.c \
onsl/libtimidity/instrum_dls.c \
onsl/libtimidity/mix.c \
onsl/libtimidity/output.c \
onsl/libtimidity/playmidi.c \
onsl/libtimidity/readmidi.c \
onsl/libtimidity/resample.c \
onsl/libtimidity/stream.c \
onsl/libtimidity/tables.c \
onsl/libtimidity/timidity.c \
onsl/LZMA.c \
onsl/ONSlaught.cpp \
onsl/OpenAL.cpp \
onsl/Options.cpp \
onsl/Plugin/LibraryLoader.cpp \
onsl/Plugin/Plugin.cpp \
onsl/ScreenSpace.cpp \
onsl/Script.cpp \
onsl/ScriptInterpreter.cpp \
onsl/sha1.cpp \
onsl/SJIS.table.cpp \
onsl/Thread.cpp \
onsl/ThreadManager.cpp \
onsl/tinyxml/tinyxml.cpp \
onsl/tinyxml/tinyxmlerror.cpp \
onsl/tinyxml/tinyxmlparser.cpp \
onsl/VariableStore.cpp \
onsl/VirtualScreen.cpp \

LOCAL_SHARED_LIBRARIES := sdl-1.2_jni sdl_imagen bz2 video_jni freetype vorbis ogg tremor mikmod flac openal timidity mpg123 

LOCAL_STATIC_LIBRARIES := jpeg png

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz -lGLESv1_CM


include $(BUILD_SHARED_LIBRARY) 
