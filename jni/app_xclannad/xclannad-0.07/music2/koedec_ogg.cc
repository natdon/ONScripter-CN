/*
 * Copyright (c) 2004-2006  Kazunori "jagarl" Ueno
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"music.h"
#include"wavfile.h"

extern int is_koe_ogg(char* head);
extern char* decode_koe_ogg(AvgKoeInfo info, int* dest_len);

extern int is_koe_ogg(char* head) {
#if HAVE_LIBVORBISFILE || HAVE_LIBVORBISIDEC
	if (strncmp(head, "OggS", 4) == 0) return 1;
	else
#endif
		return 0;
}

#if HAVE_LIBVORBISFILE || HAVE_LIBVORBISIDEC

#if HAVE_LIBVORBISFILE
#include<vorbis/vorbisfile.h>
#else /* HAVE_LIBVORBISIDEC */
#include<tremor/ivorbiscodec.h>
#include<tremor/ivorbisfile.h>
#endif

#define INITSIZE 65536

static int cur_size = 0;
static char* out = 0;
static void Resize(void) {
	char* new_out = (char*)realloc(out, cur_size+INITSIZE);
	if (new_out == 0) {
		new_out = (char*)malloc(cur_size+INITSIZE);
		memcpy(new_out, out, cur_size);
		free(out);
	}
	out = new_out;
	cur_size += INITSIZE;
}

struct OggInfo {
	FILE* stream;
	int length;
	int offset;
};
/* ogg stream 読み込み用の dummy callback */
static size_t ogg_readfunc(void* ptr, size_t size, size_t nmemb, void* datasource) {
	OggInfo* info = (OggInfo*)datasource;
	int pt = ftell(info->stream) - info->offset;
	if (pt+size*nmemb > info->length) {
		nmemb = (info->length-pt) / size;
	}
	return fread(ptr, size, nmemb, info->stream);
}
static int ogg_seekfunc(void* datasource, ogg_int64_t new_offset, int whence) {
	int pt;
	OggInfo* info = (OggInfo*)datasource;
	if (whence == SEEK_SET) pt = info->offset + new_offset;
	else if (whence == SEEK_CUR) pt = ftell(info->stream) + new_offset;
	else if (whence == SEEK_END) pt = info->offset + info->length + new_offset;
	int r = fseek(info->stream, pt, 0);
	return r;
}
static long ogg_tellfunc(void* datasource) {
	OggInfo* info = (OggInfo*)datasource;
	int pos = ftell(info->stream);
	if (pos == -1) return -1;
	return pos-info->offset;
}
static int ogg_closefunc(void* datasource) {
	return 0;
}

extern char* decode_koe_ogg(AvgKoeInfo info, int* dest_len) {
	if (info.stream == 0) return 0;
	// Voice ファイルを直接指定すると全ストリームを再生してしまうので
	// 必要な部分だけ切り出して callback 経由で帰す
	fseek(info.stream, info.offset, 0);

	ov_callbacks callback;
	callback.read_func = &ogg_readfunc;
	callback.seek_func = &ogg_seekfunc;
	callback.close_func = &ogg_closefunc;
	callback.tell_func = &ogg_tellfunc;

	OggInfo oinfo;
	oinfo.stream = info.stream;
	oinfo.length = info.length;
	oinfo.offset = info.offset;

	OggVorbis_File vf;
	int r = ov_open_callbacks((void*)&oinfo, &vf, 0, 0, callback);
	if (r != 0) {
		fprintf(stderr,"ogg stream err: %d\n",r);
		return 0;
	}
	vorbis_info* vinfo = ov_info(&vf, 0);
	info.rate = vinfo->rate;
	int channels = vinfo->channels;

	int cur = 0x2c;
	cur_size = INITSIZE;
	out = (char*)malloc(cur_size);

	int current_section;
	do {
#if HAVE_LIBVORBISFILE
		r = ov_read(&vf, out+cur, cur_size-cur, 0, 2, 1, 0); 
#else /* HAVE_LIBVORBISIDEC */
		r = ov_read(&vf, out+cur, cur_size-cur, &current_section); 
#endif
		if (r <= 0) break;
		cur += r;
		if (cur_size-INITSIZE/4 < cur) Resize();
	} while(1);
	ov_clear(&vf);

	*dest_len = cur; // うまくコンバートできてるのかなあ……
	const char* header = MakeWavHeader(info.rate, channels, 2, cur);
	memcpy(out, header, 0x2c);
	
	char* ret = out;
	out = 0;

	return ret;
}
struct OggFILE_impl {
	OggVorbis_File vf;
	ov_callbacks callback;
	OggInfo oinfo;
	OggFILE_impl(FILE*, int);
};

OggFILE_impl::OggFILE_impl(FILE* stream, int length) {
	callback.read_func = &ogg_readfunc;
	callback.seek_func = &ogg_seekfunc;
	callback.close_func = &ogg_closefunc;
	callback.tell_func = &ogg_tellfunc;
	oinfo.stream = stream;
	oinfo.length = length;
	oinfo.offset = ftell(stream);
}

OggFILE::OggFILE(FILE* stream, int len) {
	pimpl = new OggFILE_impl(stream, len);
	int r = ov_open_callbacks( (void*)&(pimpl->oinfo), &(pimpl->vf), 0, 0, pimpl->callback);
	if (r != 0) {
		delete pimpl;
		pimpl = 0;
		return;
	}
	vorbis_info* vinfo = ov_info(&(pimpl->vf), 0);
	wavinfo.SamplingRate = vinfo->rate;
	wavinfo.Channels = vinfo->channels;
	wavinfo.DataBits = 16;
}
OggFILE::~OggFILE() {
	if (pimpl) {
		ov_clear(&(pimpl->vf));
		fclose(pimpl->oinfo.stream);
		delete pimpl;
	}
}
int OggFILE::Read(char* buf, int blksize, int blklen) {
	if (pimpl == 0) return -1;
	int current_section;
#if HAVE_LIBVORBISFILE
	int r = ov_read( &(pimpl->vf), buf, blksize*blklen, 0, 2, 1, 0);
#else /* HAVE_LIBVORBISIDEC */
	int r = ov_read( &(pimpl->vf), buf, blksize*blklen, &current_section);
#endif
	if (r <= 0) { // end of file
		return -1;
	}
	while(r < blksize*blklen) {
#if HAVE_LIBVORBISFILE
		int dr = ov_read(&(pimpl->vf), buf+r, blksize*blklen-r, 0, 2, 1, 0);
#else /* HAVE_LIBVORBISIDEC */
		int dr = ov_read(&(pimpl->vf), buf+r, blksize*blklen-r, &current_section);
#endif
		if (dr <= 0) break;
		r += dr;
	}
	return r / blksize;
}
void OggFILE::Seek(int count) {
	ov_pcm_seek(&(pimpl->vf), count);
	return;
}
#else
extern char* decode_koe_ogg(AvgKoeInfo info, int* dest_len) {
	return 0;
}
OggFILE::OggFILE(FILE* stream, int a) {pimpl = 0;}
OggFILE::~OggFILE(){}
void OggFILE::Seek(int count){}
int OggFILE::Read(char* buf, int blksize, int blklen){return -1;}
#endif
