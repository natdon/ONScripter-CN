
PREF = /usr/local

CC		= arm-apple-darwin9-gcc
CXX		= arm-apple-darwin9-g++
LD		= arm-apple-darwin9-g++
RANLIB	= arm-apple-darwin9-ranlib

CFLAGS= -I. $(LOCAL_DEF) -g -O2 -DHAVE_CONFIG_H -D_THREAD_SAFE -I/usr/local/include -I/usr/local/include/smpeg -I/usr/local/include/SDL -D_GNU_SOURCE=1 -D_THREAD_SAFE -I/usr/local/include/freetype2 -pthread
CXXFLAGS	= $(CFLAGS)
LDFLAGS = `$(PREF)/bin/sdl-config --libs` \
	$(PREF)/lib/libSDL_ttf.a \
	$(PREF)/lib/libSDL_image.a \
	$(PREF)/lib/libSDL_mixer.a \
	$(PREF)/lib/libSDL.a \
	$(PREF)/lib/libsmpeg.a \
	$(PREF)/lib/libmad.a \
	$(PREF)/lib/libvorbisidec.a \
	$(PREF)/lib/libfreetype.a \
	$(PREF)/lib/libjpeg.a \
	$(PREF)/lib/libpng.a \
	-lz -lbz2 -lstdc++ \
	-F/usr/local/iphone-sysroot/System/Library/Frameworks \
	-F/usr/local/iphone-sysroot/System/Library/PrivateFrameworks \
	-framework QuartzCore -framework Foundation

SRCS	= xlovesys.cc

OBJS	 = ${SRCS:.cc=.o}

all: xclannad

xclannad: xlovesys.o system/libsystem.a scn2k/libscn2k.a window/libwindow.a font/libfont.a music2/libmusic.a
	$(LD) -o xclannad xlovesys.o system/libsystem.a scn2k/libscn2k.a window/libwindow.a font/libfont.a music2/libmusic.a $(LDFLAGS)

install: xclannad music2/nwatowav
	mkdir -p /usr/local/bin
	cp xclannad music2/nwatowav /usr/local/bin

clean:
	rm -f xclannad xlovesys ${OBJS} *.bak
	(cd font && $(MAKE) clean);
	(cd system && $(MAKE) clean);
	(cd window && $(MAKE) clean);
	(cd music2 && $(MAKE) clean);
	(cd scn2k && $(MAKE) clean);

music2/nwatowav:
	(cd music2 && $(MAKE) nwatowav);

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.cc.o:
	$(CXX) -c $(CFLAGS) -o $@ $<

font/libfont.a: FORCE
	cd font && $(MAKE) libfont.a

system/libsystem.a: FORCE
	cd system && $(MAKE) libsystem.a

scn2k/libscn2k.a: FORCE
	cd scn2k && $(MAKE) libscn2k.a

window/libwindow.a: FORCE
	cd window && $(MAKE) libwindow.a

music2/libmusic.a: FORCE
	cd music2 && $(MAKE) libmusic.a

FORCE:

