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
#include<stdlib.h>
#include<string.h>
#include <vector>
#include <list>
#include <algorithm>
#include"music.h"
#include"system/file.h"

using namespace std;

/*********************************************
**
**	FindKoe -- RealLiveの音声アーカイブ処理
**
*/

/* 声ファイルのアーカイブ用のキャッシュ */
#define koe_cache_size 7
struct AvgKoeTable {
	int koe_num;
	int length;
	int offset;
	AvgKoeTable(int _num, int _len, int _off) :
		koe_num(_num), length(_len), offset(_off) {}
	bool operator <(int number) const {
		return koe_num < number;
	}
	bool operator <(const AvgKoeTable& to) const {
		return koe_num < to.koe_num;
	}
	bool operator ==(const AvgKoeTable& to) const {
		return koe_num == to.koe_num;
	}
	bool operator ==(const int to) const {
		return koe_num == to;
	}
};
struct AvgKoeHead {
	FILE* stream;
	int file_number;
	int rate;
	KoeType type;
	vector<AvgKoeTable> table;
	AvgKoeHead(FILE* stream, int file_number, KoeType type);
	AvgKoeHead(const AvgKoeHead& from);
	~AvgKoeHead();
	AvgKoeTable* Find(int koe_num);
	bool operator !=(int num) const { return file_number != num; }
	bool operator ==(int num) const { return file_number == num; }
};
struct AvgKoeCache {
	list<AvgKoeHead> cache;
	AvgKoeInfo Find(int file_number, int index);
};
static AvgKoeCache koe_cache;

AvgKoeInfo FindKoe(int file_number, int index) {
	return koe_cache.Find(file_number, index);
};

AvgKoeInfo AvgKoeCache::Find(int file_number, int index) {
	AvgKoeInfo info;
	info.stream = 0; info.length = 0; info.offset = 0;

	list<AvgKoeHead>::iterator it;
	it = find(cache.begin(), cache.end(), file_number);
	if (it == cache.end()) {
		/* 新たに head を作る */
		char fname[100];
		KoeType type = koe_unknown;
		sprintf(fname, "z%03d.koe", file_number);
		ARCINFO* arcinfo = file_searcher.Find(FILESEARCH::KOE,fname,".koe");
		if (arcinfo == 0) {
			type = koe_nwk;
			sprintf(fname, "z%04d.nwk", file_number);
			arcinfo = file_searcher.Find(FILESEARCH::KOE,fname,".nwk");
		}
		if (arcinfo == 0) {
			type = koe_ovk;
			sprintf(fname, "z%04d.ovk", file_number);
			arcinfo = file_searcher.Find(FILESEARCH::KOE,fname,".ovk");
		}
		if (arcinfo == 0) return info;
		FILE* stream = arcinfo->OpenFile();
		delete arcinfo;
		if (stream == 0) return info;
		cache.push_front(AvgKoeHead(stream, file_number, type));
		if (cache.size() >= koe_cache_size) cache.pop_back();
		it = cache.begin();
	}
	if (it->file_number != file_number) return info; // 番号がおかしい
	AvgKoeTable* table = it->Find(index);
	if (table == 0) return info; // index が見付からない
	// info を作成する
	info.length = table->length;
	info.offset = table->offset;
	info.rate = it->rate;
	info.type = it->type;
#ifdef ENABLE_PATCH
#if defined(PSP) || defined(WINCE)
	int new_fd = -1;
#else
	int new_fd = dup(fileno(it->stream));
#endif
#endif
	if (new_fd == -1) info.stream = 0;
	else info.stream = fdopen(new_fd, "rb");
	return info;
}

AvgKoeHead::AvgKoeHead(const AvgKoeHead& from) {
	if (from.stream) {
#ifdef ENABLE_PATCH
#if defined(PSP) || defined(WINCE)
		int new_fd = -1;
#else
		int new_fd = dup(fileno(from.stream));
#endif
#endif
		if (new_fd == -1) stream = 0;
		else stream = fdopen(new_fd, "rb");
	}
	file_number = from.file_number;
	rate = from.rate;
	table = from.table;
	type = from.type;
}
AvgKoeHead::AvgKoeHead(FILE* _s, int _file_number, KoeType _type) {
	char head[0x20];
	stream = _s; file_number = _file_number;
	int offset = ftell(stream);
	rate = 22050;
	type = _type;
	if (stream == 0) return;
	/* header 読み込み */
	if (type == koe_nwk) { // 新しい形式 : .nwk file
		rate = 44100;
		fread(head, 4, 1, stream);
		int table_len = read_little_endian_int(head);
		table.reserve(table_len);
		int i;
		for (i=0; i<table_len; i++) {
			fread(head, 12, 1, stream);
			int sz = read_little_endian_int(head);
			int off = read_little_endian_int(head+4);
			int cnt = read_little_endian_int(head+8);
			table.push_back(AvgKoeTable(cnt, sz, off));
		}
	} else if (type == koe_ovk) { // Little Busters! : .ovk file
		rate = 44100;
		fread(head, 4, 1, stream);
		int table_len = read_little_endian_int(head);
		table.reserve(table_len);
		int i;
		for (i=0; i<table_len; i++) {
			fread(head, 16, 1, stream);
			int sz = read_little_endian_int(head);
			int off = read_little_endian_int(head+4);
			int cnt = read_little_endian_int(head+8);
			table.push_back(AvgKoeTable(cnt, sz, off));
		}
	} else { // .koe file
		fread(head, 0x20, 1, stream);
		if (strncmp(head, "KOEPAC", 7) != 0) { // invalid header
			stream = 0;
			return;
		}
		int table_len = read_little_endian_int(head+0x10);
		rate = read_little_endian_int(head+0x18);
		if (rate == 0) rate = 22050;
		/* table 読み込み */
		table.reserve(table_len);
		char* buf = new char[table_len*8];
		fread(buf, table_len, 8, stream);
		int i; for (i=0; i<table_len; i++) {
			int cnt = read_little_endian_short(buf+i*8);
			int sz  = read_little_endian_short(buf+i*8+2);
			int off = read_little_endian_int(buf+i*8+4);
			table.push_back(AvgKoeTable(cnt, sz, off));
		}
	}
	sort(table.begin(), table.end());
}

AvgKoeHead::~AvgKoeHead(void) {
	if (stream) fclose(stream);
	stream = 0;
}
AvgKoeTable* AvgKoeHead::Find(int koe_num) {
	if (table.empty()) return 0;
	vector<AvgKoeTable>::iterator it;
	it = lower_bound(table.begin(), table.end(), koe_num);
	if (it == table.end() || it->koe_num != koe_num) return 0;
	return &table[it-table.begin()];
}

/*********************************************
**
**	MakeWavHeader : koe ファイルに wave header を付ける
**
*/

static unsigned char orig_header[0x2c] = {
	0x52, 0x49, 0x46, 0x46,	/* +00 "RIFF" */
	0x00, 0x00, 0x00, 0x00, /* +04 file size - 8 */
	0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20, /* +08 "WAVEfmt " */
	0x10, 0x00, 0x00, 0x00, /* +10 fmt size */
	0x01, 0x00,             /* +14 wFormatTag */
	0x02, 0x00,             /* +16 Channels */
	0x44, 0xac, 0x00, 0x00, /* +18 rate */
	0x10, 0xb1, 0x02, 0x00, /* +1c BytesPerSec = rate * BlockAlign */
	0x04, 0x00,             /* +20 BlockAlign = channels*BytesPerSample */
	0x10, 0x00,             /* +22 BitsPerSample */
	0x64, 0x61, 0x74, 0x61, /* +24 "data" */
	0x00, 0x00, 0x00, 0x00  /* +28 filesize - 0x2c */
};

const char* MakeWavHeader(int rate, int ch, int bps, int size) {
	static char header[0x2c];
	memcpy(header, (const char*)orig_header, 0x2c);
	write_little_endian_int(header+0x04, size-8);
	write_little_endian_int(header+0x28, size-0x2c);
	write_little_endian_int(header+0x18, rate);
	write_little_endian_int(header+0x1c, rate*ch*bps);
	header[0x16] = ch;
	header[0x20] = ch*bps;
	header[0x22] = bps*8;
	return header;
}

/*********************************************
**
**	decode_koe -- 音声データ展開
**
*/

extern char* decode_koe(AvgKoeInfo info, int* len);

/* 8bit -> 16bit への変換テーブル。本来は signed short だが
** とりあえず unsigned で扱っている
*/

unsigned short koe_8bit_trans_tbl[256] = {
  0x8000,0x81ff,0x83f9,0x85ef,0x87e1,0x89cf,0x8bb9,0x8d9f,
  0x8f81,0x915f,0x9339,0x950f,0x96e1,0x98af,0x9a79,0x9c3f,
  0x9e01,0x9fbf,0xa179,0xa32f,0xa4e1,0xa68f,0xa839,0xa9df,
  0xab81,0xad1f,0xaeb9,0xb04f,0xb1e1,0xb36f,0xb4f9,0xb67f,
  0xb801,0xb97f,0xbaf9,0xbc6f,0xbde1,0xbf4f,0xc0b9,0xc21f,
  0xc381,0xc4df,0xc639,0xc78f,0xc8e1,0xca2f,0xcb79,0xccbf,
  0xce01,0xcf3f,0xd079,0xd1af,0xd2e1,0xd40f,0xd539,0xd65f,
  0xd781,0xd89f,0xd9b9,0xdacf,0xdbe1,0xdcef,0xddf9,0xdeff,
  0xe001,0xe0ff,0xe1f9,0xe2ef,0xe3e1,0xe4cf,0xe5b9,0xe69f,
  0xe781,0xe85f,0xe939,0xea0f,0xeae1,0xebaf,0xec79,0xed3f,
  0xee01,0xeebf,0xef79,0xf02f,0xf0e1,0xf18f,0xf239,0xf2df,
  0xf381,0xf41f,0xf4b9,0xf54f,0xf5e1,0xf66f,0xf6f9,0xf77f,
  0xf801,0xf87f,0xf8f9,0xf96f,0xf9e1,0xfa4f,0xfab9,0xfb1f,
  0xfb81,0xfbdf,0xfc39,0xfc8f,0xfce1,0xfd2f,0xfd79,0xfdbf,
  0xfe01,0xfe3f,0xfe79,0xfeaf,0xfee1,0xff0f,0xff39,0xff5f,
  0xff81,0xff9f,0xffb9,0xffcf,0xffe1,0xffef,0xfff9,0xffff,
  0x0000,0x0001,0x0007,0x0011,0x001f,0x0031,0x0047,0x0061,
  0x007f,0x00a1,0x00c7,0x00f1,0x011f,0x0151,0x0187,0x01c1,
  0x01ff,0x0241,0x0287,0x02d1,0x031f,0x0371,0x03c7,0x0421,
  0x047f,0x04e1,0x0547,0x05b1,0x061f,0x0691,0x0707,0x0781,
  0x07ff,0x0881,0x0907,0x0991,0x0a1f,0x0ab1,0x0b47,0x0be1,
  0x0c7f,0x0d21,0x0dc7,0x0e71,0x0f1f,0x0fd1,0x1087,0x1141,
  0x11ff,0x12c1,0x1387,0x1451,0x151f,0x15f1,0x16c7,0x17a1,
  0x187f,0x1961,0x1a47,0x1b31,0x1c1f,0x1d11,0x1e07,0x1f01,
  0x1fff,0x2101,0x2207,0x2311,0x241f,0x2531,0x2647,0x2761,
  0x287f,0x29a1,0x2ac7,0x2bf1,0x2d1f,0x2e51,0x2f87,0x30c1,
  0x31ff,0x3341,0x3487,0x35d1,0x371f,0x3871,0x39c7,0x3b21,
  0x3c7f,0x3de1,0x3f47,0x40b1,0x421f,0x4391,0x4507,0x4681,
  0x47ff,0x4981,0x4b07,0x4c91,0x4e1f,0x4fb1,0x5147,0x52e1,
  0x547f,0x5621,0x57c7,0x5971,0x5b1f,0x5cd1,0x5e87,0x6041,
  0x61ff,0x63c1,0x6587,0x6751,0x691f,0x6af1,0x6cc7,0x6ea1,
  0x707f,0x7261,0x7447,0x7631,0x781f,0x7a11,0x7c07,0x7fff
};

/* ADPCM・・・じゃないらしい。ただのDPCMのナめたテーブル。
** 自動生成すりゃいいんだけど256byteだったら
** テーブルでも問題ないでしょ
*/

char koe_ad_trans_tbl[256] = {
  0x00,0xff,0x01,0xfe,0x02,0xfd,0x03,0xfc,0x04,0xfb,0x05,0xfa,0x06,0xf9,0x07,0xf8,
  0x08,0xf7,0x09,0xf6,0x0a,0xf5,0x0b,0xf4,0x0c,0xf3,0x0d,0xf2,0x0e,0xf1,0x0f,0xf0,
  0x10,0xef,0x11,0xee,0x12,0xed,0x13,0xec,0x14,0xeb,0x15,0xea,0x16,0xe9,0x17,0xe8,
  0x18,0xe7,0x19,0xe6,0x1a,0xe5,0x1b,0xe4,0x1c,0xe3,0x1d,0xe2,0x1e,0xe1,0x1f,0xe0,
  0x20,0xdf,0x21,0xde,0x22,0xdd,0x23,0xdc,0x24,0xdb,0x25,0xda,0x26,0xd9,0x27,0xd8,
  0x28,0xd7,0x29,0xd6,0x2a,0xd5,0x2b,0xd4,0x2c,0xd3,0x2d,0xd2,0x2e,0xd1,0x2f,0xd0,
  0x30,0xcf,0x31,0xce,0x32,0xcd,0x33,0xcc,0x34,0xcb,0x35,0xca,0x36,0xc9,0x37,0xc8,
  0x38,0xc7,0x39,0xc6,0x3a,0xc5,0x3b,0xc4,0x3c,0xc3,0x3d,0xc2,0x3e,0xc1,0x3f,0xc0,
  0x40,0xbf,0x41,0xbe,0x42,0xbd,0x43,0xbc,0x44,0xbb,0x45,0xba,0x46,0xb9,0x47,0xb8,
  0x48,0xb7,0x49,0xb6,0x4a,0xb5,0x4b,0xb4,0x4c,0xb3,0x4d,0xb2,0x4e,0xb1,0x4f,0xb0,
  0x50,0xaf,0x51,0xae,0x52,0xad,0x53,0xac,0x54,0xab,0x55,0xaa,0x56,0xa9,0x57,0xa8,
  0x58,0xa7,0x59,0xa6,0x5a,0xa5,0x5b,0xa4,0x5c,0xa3,0x5d,0xa2,0x5e,0xa1,0x5f,0xa0,
  0x60,0x9f,0x61,0x9e,0x62,0x9d,0x63,0x9c,0x64,0x9b,0x65,0x9a,0x66,0x99,0x67,0x98,
  0x68,0x97,0x69,0x96,0x6a,0x95,0x6b,0x94,0x6c,0x93,0x6d,0x92,0x6e,0x91,0x6f,0x90,
  0x70,0x8f,0x71,0x8e,0x72,0x8d,0x73,0x8c,0x74,0x8b,0x75,0x8a,0x76,0x89,0x77,0x88,
  0x78,0x87,0x79,0x86,0x7a,0x85,0x7b,0x84,0x7c,0x83,0x7d,0x82,0x7e,0x81,0x7f,0x80
};

extern int is_koe_ogg(char* head);
extern char* decode_koe_ogg(AvgKoeInfo info, int* dest_len);

char* decode_koe(AvgKoeInfo info, int* dest_len) {
	char buf[1024]; char* table;
	unsigned char* src_orig, *src;
	unsigned short* dest_orig, *dest;
	int all_len; int i,j;
	if (info.stream == NULL) {
		return NULL;
	}
	fseek(info.stream, info.offset, 0);
	if (info.type == koe_nwk) {
		return decode_koe_nwa(info, dest_len);
	}
	fread(buf, 1, 0x20, info.stream);
	if (is_koe_ogg(buf)) {
		fseek(info.stream, -20, 1);
		return decode_koe_ogg(info, dest_len);
	}
	/* avg32 の声データ展開 */
	table = (char*)malloc(info.length*2);
	fseek(info.stream, info.offset, 0);
	fread(table, 2, info.length, info.stream);

	all_len = 0;
	for (i=0; i<info.length; i++)
		all_len += read_little_endian_short(table + i*2);
	/* データ読み込み */
	src_orig  = (unsigned char*) malloc(all_len);
	dest_orig = (unsigned short*)malloc(info.length * 0x1000 + 0x2c);
	if (src_orig == NULL || dest_orig == NULL) return NULL;
	src = src_orig;
	fread(src, 1, all_len, info.stream);
	*dest_len = info.length * 0x400 * 4;
	const char* header = MakeWavHeader(info.rate, 2, 2, *dest_len);
	memcpy(dest_orig, header, 0x2c);
	dest = dest_orig + 0x2c;
	/* memset(dest_data, 0, table_len * 0x1000); */
	
	/* 展開 */
	for (i=0; i<info.length; i++) {
		int slen = read_little_endian_short(table+i*2);
		if (slen == 0) { // do nothing
			memset(dest, 0, 0x1000);
			dest += 0x800; src += 0;
		} else if (slen == 0x400) { // table 変換
			for (j=0; j<0x400; j++) {
				write_little_endian_short((char*)(dest+0), koe_8bit_trans_tbl[*src]);
				write_little_endian_short((char*)(dest+1), koe_8bit_trans_tbl[*src]);
				dest += 2; src++;
			}
		} else { // DPCM
			char d = 0; short o2;
			int k,j; for (j=0, k=0; j<slen && k < 0x800; j++) {
				unsigned char s = src[j];
				if ( (s+1) & 0x0f) {
					d -= koe_ad_trans_tbl[s & 0x0f];
				} else {
					unsigned char s2;
					s >>= 4; s &= 0x0f; s2 = s;
					s = src[++j]; s2 |= (s<<4) & 0xf0;
					d -= koe_ad_trans_tbl[s2];
				}
				o2 = koe_8bit_trans_tbl[ (unsigned char)d];
				write_little_endian_short((char*)(dest+k), o2);
				write_little_endian_short((char*)(dest+k+1), o2);
				k+=2;
				s >>= 4;
				if ((s+1) & 0x0f) {
					d -= koe_ad_trans_tbl[s & 0x0f];
				} else {
					d -= koe_ad_trans_tbl[ src[++j] ];
				}
				o2 = koe_8bit_trans_tbl[ (unsigned char)d];
				write_little_endian_short((char*)(dest+k), o2);
				write_little_endian_short((char*)(dest+k+1), o2);
				k+=2;
			}
			dest += 0x800; src += slen;
		}
	}
	free( (void*) table);
	free( (void*) src_orig);
	return (char*)dest_orig;
}


