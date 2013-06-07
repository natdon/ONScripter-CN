

CC  = gcc
CXX = g++
LD  = g++

OBJ_ABLEND = 
OBJ_AUDIO = 
OBJ_CDROM =
OBJ_FONT =  font_freetype2.o cp932tojis0213.o
OBJ_GRAPHDEV = sdl_video.o sdl_mode.o sdl_draw.o sdl_event.o sdl_image.o sdl_cursor.o image.o
OBJ_MENU = menu_null.o
OBJ_MIDI = 
OBJ_SYS35CMD = cmdb.o cmdc.o cmdd.o cmde.o cmdf.o cmdg.o cmdh.o cmdi.o cmdj.o cmdk.o cmdl.o cmdm.o cmdn.o cmdo.o cmdp.o cmdq.o cmds.o cmdt.o cmdu.o cmdv.o cmdw.o cmdy.o cmdz.o cmd2F.o cmd2F60.o
OBJ_CG = pms.o vsp.o bmp.o qnt.o
OBJ_MUS = music.o music_client.o wavmix.o wavfile.o musstream.o pcmlib.o musstream_ogg.o musstream_wav_ima.o musstream_wav.o #midi.o audio.o cdrom.o 
OBJ_MISC = LittleEndian.o counter.o imput.o profile.o mt19937-1.o joystick_dmy.o eucsjis.o filecheck.o
OBJ_SCO = cali.o scenario.o variable.o cmd_check.o hankana2sjis.o nact.o selection.o message.o savedata.o hankaku.o s39ain.o s39init.o
OBJ_GRAPH = ags.o font.o flood.o cg.o ecopy.o antialiase.o alpha_plane.o cursor.o
OBJ_NET = network.o
OBJ_DLL = 
OBJ_XSYSTEM35 = xsystem35.o dri.o ald_manager.o cache.o
OBJ_EXT = 

OBJS = $(OBJ_XSYSTEM35) $(OBJ_ABLEND) $(OBJ_AUDIO) $(OBJ_CDROM) $(OBJ_FONT) $(OBJ_GRAPHDEV) $(OBJ_MENU) $(OBJ_MIDI) $(OBJ_SYS35CMD) $(OBJ_CG) $(OBJ_SCO) $(OBJ_MUS) $(OBJ_GRAPH) $(OBJ_MISC) $(OBJ_NET) $(OBJ_DLL) $(OBJ_EXT)

DEF = -fsigned-char -DHAVE_CONFIG_H -D_GNU_SOURCE=1 -DDEPTH=16 -DCACHE_TOTALSIZE=20 -DMODULE_PATH=\".\" -DENABLE_SDL=1

INC    = -I.
CFLAGS = `freetype-config --cflags` `sdl-config --cflags` $(DEF) $(INC) -g
LIBS   = `freetype-config --libs` -lSDL_mixer `sdl-config --libs` -lvorbisidec -lmad -lz

EXESUFFIX = 

APPNAME = xsystem35

TARGET = $(APPNAME)$(EXESUFFIX)

all : $(TARGET)

.cpp.o:
	$(CXX) $(CFLAGS) -o $@ -c $< 

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

$(APPNAME)$(EXESUFFIX): $(OBJS)
	$(LD) -o $(APPNAME)$(EXESUFFIX) $(OBJS) $(LIBS)

clean:
	rm $(OBJS)
	rm $(TARGET)
