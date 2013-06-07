LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := $(lastword $(subst /, ,$(LOCAL_PATH)))

APP_SUBDIR := $(patsubst $(LOCAL_PATH)/%, %, $(LOCAL_PATH)/xsystem35-1.7.3-pre5/src)

# Add more subdirs here, like src/subdir1 src/subdir2

LOCAL_CFLAGS := $(foreach D, $(APP_SUBDIR), -I$(LOCAL_PATH)/$(D)) \
				-I$(LOCAL_PATH)/../ \
				-I$(LOCAL_PATH)/../sdl-1.2_jni/include \
				-I$(LOCAL_PATH)/../sdl_mixer/include \
				-I$(LOCAL_PATH)/../sdl_image/include \
				-I$(LOCAL_PATH)/../sdl_ttf/include \
				-I$(LOCAL_PATH)/../freetype/include \
				-I$(LOCAL_PATH)/../jpeg \
				-I$(LOCAL_PATH)/../png \
				-I$(LOCAL_PATH)/../video_jni\
				-fsigned-char -DHAVE_CONFIG_H -D_GNU_SOURCE=1 -DDEPTH=16 -DCACHE_TOTALSIZE=16 -DENABLE_SDL=1

#Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cpp

SRC_ABLEND = 
SRC_AUDIO = 
SRC_CDROM = 
SRC_FONT = font_freetype2.c cp932tojis0213.c
SRC_GRAPHDEV = sdl_video.c sdl_mode.c sdl_draw.c sdl_event.c sdl_image.c sdl_cursor.c image.c
SRC_MENU = menu_null.c
SRC_MIDI = 
SRC_SYS35CMD = cmdb.c cmdc.c cmdd.c cmde.c cmdf.c cmdg.c cmdh.c cmdi.c cmdj.c cmdk.c cmdl.c cmdm.c cmdn.c cmdo.c cmdp.c cmdq.c cmds.c cmdt.c cmdu.c cmdv.c cmdw.c cmdy.c cmdz.c cmd2F.c cmd2F60.c
SRC_CG = pms.c vsp.c bmp.c qnt.c
SRC_MUS = music.c music_client.c wavfile.c wavmix.c musstream.c pcmlib.c musstream_wav.c musstream_ogg.c musstream_wav_ima.c
SRC_MISC = LittleEndian.c counter.c imput.c profile.c mt19937-1.c joystick_dmy.c eucsjis.c filecheck.c 
SRC_SCO = cali.c scenario.c variable.c cmd_check.c hankana2sjis.c nact.c selection.c message.c savedata.c hankaku.c s39ain.c s39init.c
SRC_GRAPH = ags.c font.c flood.c cg.c ecopy.c antialiase.c alpha_plane.c cursor.c
SRC_NET = network.c
SRC_DLL = 
SRC_EXT = 
SRC_XSYSTEM35 = xsystem35.c dri.c ald_manager.c cache.c
SRC_ALL = $(SRC_ABLEND) $(SRC_AUDIO) $(SRC_CDROM) $(SRC_FONT) $(SRC_GRAPHDEV) $(SRC_MENU) $(SRC_MIDI) $(SRC_SYS35CMD) $(SRC_CG) $(SRC_MUS) $(SRC_MISC) $(SRC_SCO) $(SRC_GRAPH) $(SRC_NET) $(SRC_DLL) $(SRC_EXT) $(SRC_XSYSTEM35)

LOCAL_SRC_FILES := $(addprefix $(APP_SUBDIR)/,$(SRC_ALL))

LOCAL_SHARED_LIBRARIES := sdl-1.2_jni sdl_mixer sdl_ttf sdl_image mad freetype video_jni
LOCAL_STATIC_LIBRARIES := 

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz -lGLESv1_CM

include $(BUILD_SHARED_LIBRARY)
