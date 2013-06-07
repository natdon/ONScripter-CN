/*
* Copyright (c) 2008-2011, Helios (helios.vmg@gmail.com)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, 
*       this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of the author may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*     * Products derived from this software may not be called "ONSlaught" nor
*       may "ONSlaught" appear in their names without specific prior written
*       permission from the author. 
*
* THIS SOFTWARE IS PROVIDED BY HELIOS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL HELIOS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Functions.h"
#include "ThreadManager.h"
#include "IOFunctions.h"
#include "enums.h"
#include "tinyxml/tinyxml.h"
#include <bzlib.h>
#include <cassert>
#if NONS_SYS_WINDOWS
#include <windows.h>
#endif

bool getbit(void *arr,ulong *byteoffset,uchar *bitoffset){
	uchar *array=(uchar *)arr;
	bool res=(array[*byteoffset]>>(7-*bitoffset))&1;
	(*bitoffset)++;
	if (*bitoffset>7){
		(*byteoffset)++;
		*bitoffset=0;
	}
	return res;
}

ulong getbits(void *arr,uchar bits,ulong *byteoffset,uchar *bitoffset){
	uchar *array=(uchar *)arr;
	ulong res=0;
	if (bits>sizeof(ulong)*8)
		bits=sizeof(ulong)*8;
	for (;bits>0;bits--){
		res<<=1;
		res|=(ulong)getbit(array,byteoffset,bitoffset);
	}
	return res;
}

Uint32 secondsSince1970(){
	return (Uint32)time(0);
}

char checkNativeEndianness(){
	Uint16 a=0x1234;
	if (*(uchar *)&a==0x12)
		return NONS_BIG_ENDIAN;
	else
		return NONS_LITTLE_ENDIAN;
}

Uint8 readByte(const void *buffer,ulong &offset){
	return ((uchar *)buffer)[offset++];
}

Sint16 readSignedWord(const void *_buffer,ulong &offset){
	uchar *buffer=(uchar *)_buffer;
	Sint16 r=0;
	for (char a=1;a>=0;a--){
		r<<=8;
		r|=buffer[offset+a];
	}
	offset+=2;
	return r;
}

Uint16 readWord(const void *_buffer,ulong &offset){
	uchar *buffer=(uchar *)_buffer;
	Uint16 r=0;
	for (int a=1;a>=0;a--){
		r<<=8;
		r|=buffer[offset+a];
	}
	offset+=2;
	return r;
}

Sint32 readSignedDWord(const void *_buffer,ulong &offset){
	uchar *buffer=(uchar *)_buffer;
	Sint32 r=0;
	for (int a=3;a>=0;a--){
		r<<=8;
		r|=buffer[offset+a];
	}
	offset+=4;
	return r;
}

Uint32 readDWord(const void *_buffer,ulong &offset){
	uchar *buffer=(uchar *)_buffer;
	Uint32 r=0;
	for (int a=3;a>=0;a--){
		r<<=8;
		r|=buffer[offset+a];
	}
	offset+=4;
	return r;
}

Uint64 readQWord(const void *_buffer,ulong &offset){
	uchar *buffer=(uchar *)_buffer;
	Uint64 r=0;
	for (int a=7;a>=0;a--){
		r<<=8;
		r|=buffer[offset+a];
	}
	offset+=8;
	return r;
}

std::string readString(const void *buffer,ulong &offset){
	std::string r(((char *)buffer)+offset);
	offset+=r.size()+1;
	return r;
}

void writeByte(Uint8 a,std::vector<uchar> &str,ulong offset){
	if (offset==ULONG_MAX)
		str.push_back(a&0xFF);
	else
		str[offset]=a&0xFF;
}

void writeWord(Uint16 a,std::vector<uchar> &str,ulong offset){
	ulong off=(offset==ULONG_MAX)?str.size():offset;
	for (ulong b=0;b<2;b++,off++){
		if (str.size()>off)
			str[off]=a&0xFF;
		else
			str.push_back(a&0xFF);
		a>>=8;
	}
}

void writeDWord(Uint32 a,std::vector<uchar> &str,ulong offset){
	ulong off=(offset==ULONG_MAX)?str.size():offset;
	for (ulong b=0;b<4;b++,off++){
		if (str.size()>off)
			str[off]=a&0xFF;
		else
			str.push_back(a&0xFF);
		a>>=8;
	}
}

void writeWordBig(Uint16 a,std::vector<uchar> &str,ulong offset){
	if (offset==ULONG_MAX)
		offset=str.size();
	for (ulong b=0;b<2;b++,offset++){
		if (str.size()>offset)
			str[offset]=a>>8;
		else
			str.push_back(a>>8);
		a<<=8;
	}
}

void writeDWordBig(Uint32 a,std::vector<uchar> &str,ulong offset){
	if (offset==ULONG_MAX)
		offset=str.size();
	for (ulong b=0;b<4;b++,offset++){
		if (str.size()>offset)
			str[offset]=a>>24;
		else
			str.push_back(a>>24);
		a<<=8;
	}
}

void writeString(const std::wstring &a,std::vector<uchar> &str){
	std::string s=UniToUTF8(a);
	str.insert(str.end(),s.begin(),s.end());
	str.push_back(0);
}

std::wstring readline(std::wstring::const_iterator start,std::wstring::const_iterator end,std::wstring::const_iterator *out){
	std::wstring::const_iterator end2=start;
	for (;end2!=end && *end2!=13 && *end2!=10;end2++);
	if (!!out){
		*out=end2;
		for (;*out!=end && (**out==13 || **out==10);(*out)++);
	}
	return std::wstring(start,end2);
}

ErrorCode inPlaceDecryption(void *vbuffer,size_t length,ulong mode){
	uchar *buffer=(uchar *)vbuffer;
	switch (mode){
		case ENCRYPTION::NONE:
		default:
			return NONS_NO_ERROR;
		case ENCRYPTION::XOR84:
			for (ulong a=0;a<length;a++)
				buffer[a]^=0x84;
			return NONS_NO_ERROR;
		case ENCRYPTION::VARIABLE_XOR:
			{
				uchar magic_numbers[5]={0x79,0x57,0x0d,0x80,0x04};
				ulong index=0;
				for (ulong a=0;a<length;a++){
					buffer[a]^=magic_numbers[index];
					index=(index+1)%5;
				}
				return NONS_NO_ERROR;
			}
		case ENCRYPTION::TRANSFORM_THEN_XOR84:
			{
				o_stderr <<"TRANSFORM_THEN_XOR84 (aka mode 4) encryption not implemented for a very good\n"
					"reason. Which I, of course, don\'t need to explain to you. Good day.";
				return NONS_NOT_IMPLEMENTED;
			}
	}
	return NONS_NO_ERROR;
}

#if NONS_SYS_WINDOWS
extern HWND mainWindow;

BOOL CALLBACK findMainWindow_callback(HWND handle,LPARAM lparam){
	const std::wstring *caption=(const std::wstring *)lparam;
	std::wstring buffer=*caption;
	GetWindowText(handle,&buffer[0],buffer.size());
	if (buffer!=*caption)
		return 1;
	mainWindow=handle;
	return 0;
}

void findMainWindow(const wchar_t *caption){
	std::wstring string=caption;
	EnumWindows(findMainWindow_callback,(LPARAM)&string);
}
#endif

extern wchar_t SJIS2Unicode[0x10000],
	Unicode2SJIS[0x10000];

#define SINGLE_CHARACTER_F(name) inline bool name(wchar_t &dst,const uchar *src,size_t size,ulong &bytes_used)

SINGLE_CHARACTER_F(ConvertSingleUTF8Character){
	bytes_used=0;
	uchar byte=src[bytes_used++];
	wchar_t c=0;
	if (!(byte&0x80))
		c=byte;
	else if ((byte&0xC0)==0x80){
		bytes_used=1;
		return 0;
	}else if ((byte&0xE0)==0xC0){
		if (size<2){
			bytes_used=size;
			return 0;
		}
		c=byte&0x1F;
		c<<=6;
		c|=src[bytes_used++]&0x3F;
	}else if ((byte&0xF0)==0xE0){
		if (size<3){
			bytes_used=size;
			return 0;
		}
		c=byte&0x0F;
		c<<=6;
		c|=src[bytes_used++]&0x3F;
		c<<=6;
		c|=src[bytes_used++]&0x3F;
	}else if ((byte&0xF8)==0xF0){
		if (size<4){
			bytes_used=size;
			return 0;
		}
#if WCHAR_MAX==0xFFFF
		c='?';
#else
		c=byte&0x07;
		c<<=6;
		c|=src[bytes_used++]&0x3F;
		c<<=6;
		c|=src[bytes_used++]&0x3F;
		c<<=6;
		c|=src[bytes_used++]&0x3F;
#endif
	}
	dst=c;
	return 1;
}

#define IS_SJIS_WIDE(x) ((x)>=0x81 && (x)<=0x9F || (x)>=0xE0 && (x)<=0xEF)

SINGLE_CHARACTER_F(ConvertSingleSJISCharacter){
	bytes_used=0;
	uchar c0=src[bytes_used++];
	wchar_t c1;
	if (IS_SJIS_WIDE(c0)){
		if (size<2){
			bytes_used=size;
			return 0;
		}

		c1=(c0<<8)|src[bytes_used++];
	}else
		c1=c0;
	if (SJIS2Unicode[c1]=='?' && c1!='?'){
		o_stderr <<"ENCODING ERROR: Character SJIS+"<<itohexc(c1,4)
			<<" is unsupported by this Shift JIS->Unicode implementation. Replacing with '?'.\n";
	}
	dst=SJIS2Unicode[c1];
	return 1;
}

void ConvertSingleCharacterToUTF8(char dst[4],wchar_t src,ulong &bytes_used){
	bytes_used=0;
	if (src<0x80)
		dst[bytes_used++]=(char)src;
	else if (src<0x800){
		dst[bytes_used++]=(src>>6)|0xC0;
		dst[bytes_used++]=src&0x3F|0x80;
#if WCHAR_MAX==0xFFFF
	}else{
#else
	}else if (src<0x10000){
#endif
		dst[bytes_used++]=(src>>12)|0xE0;
		dst[bytes_used++]=((src>>6)&0x3F)|0x80;
		dst[bytes_used++]=src&0x3F|0x80;
#if WCHAR_MAX==0xFFFF
	}
#else
	}else{
		dst[bytes_used++]=(src>>18)|0xF0;
		dst[bytes_used++]=((src>>12)&0x3F)|0x80;
		dst[bytes_used++]=((src>>6)&0x3F)|0x80;
		dst[bytes_used++]=src&0x3F|0x80;
	}
#endif
}

void ISO_WC(wchar_t *dst,const uchar *src,ulong srcl){
	for (ulong a=0;a<srcl;a++,src++,dst++)
		*dst=*src;
}

void UTF8_WC(wchar_t *dst,const uchar *src,ulong srcl){
	ulong advance;
	for (ulong a=0;a<srcl;a+=advance)
		if (!ConvertSingleUTF8Character(*dst++,src+a,srcl-a,advance))
			break;
}

ulong SJIS_WC(wchar_t *dst,const uchar *src,ulong srcl){
	ulong ret=0;
	ulong advance;
	for (ulong a=0;a<srcl;a+=advance,ret++)
		if (!ConvertSingleSJISCharacter(*dst++,src+a,srcl-a,advance))
			break;
	return ret;
}

void WC_88591(uchar *dst,const wchar_t *src,ulong srcl){
	for (ulong a=0;a<srcl;a++,src++,dst++)
		*dst=(*src>0xFF)?'?':*src;
}

ulong getUTF8size(const wchar_t *buffer,ulong size){
	ulong res=0;
	for (ulong a=0;a<size;a++){
		if (buffer[a]<0x80)
			res++;
		else if (buffer[a]<0x800)
			res+=2;
#if WCHAR_MAX==0xFFFF
		else
#else
		else if (buffer[a]<0x10000)
#endif
			res+=3;
#if WCHAR_MAX!=0xFFFF
		else
			res+=4;
#endif
	}
	return res;
}

void WC_UTF8(uchar *dst,const wchar_t *src,ulong srcl){
	for (ulong a=0;a<srcl;a++){
		wchar_t character=*src++;
		if (character<0x80)
			*dst++=(uchar)character;
		else if (character<0x800){
			*dst++=(character>>6)|0xC0;
			*dst++=character&0x3F|0x80;
#if WCHAR_MAX==0xFFFF
		}else{
#else
		}else if (character<0x10000){
#endif
			*dst++=(character>>12)|0xE0;
			*dst++=((character>>6)&0x3F)|0x80;
			*dst++=character&0x3F|0x80;
#if WCHAR_MAX==0xFFFF
		}
#else
		}else{
			*dst++=(character>>18)|0xF0;
			*dst++=((character>>12)&0x3F)|0x80;
			*dst++=((character>>6)&0x3F)|0x80;
			*dst++=character&0x3F|0x80;
		}
#endif
	}
}

ulong WC_SJIS(uchar *dst,const wchar_t *src,ulong srcl){
	ulong ret=0;
	for (ulong a=0;a<srcl;a++){
		wchar_t srcc=*src++,
			character=Unicode2SJIS[srcc];
		if (character=='?' && srcc!='?'){
			o_stderr <<"ENCODING ERROR: Character U+"<<itohexc(srcc,4)
				<<" is unsupported by this Unicode->Shift JIS implementation. Replacing with '?'.\n";
		}
		if (character<0x100)
			dst[ret++]=(uchar)character;
		else{
			dst[ret++]=character>>8;
			dst[ret++]=character&0xFF;
		}
	}
	return ret;
}

std::string UniToISO88591(const std::wstring &str){
	std::string res;
	res.resize(str.size());
	WC_88591((uchar *)&res[0],&str[0],str.size());
	return res;
}

std::string UniToUTF8(const std::wstring &str,bool addBOM){
	std::string res;
	res.resize(getUTF8size(&str[0],str.size())+int(addBOM)*3);
	if (addBOM){
		res.push_back(BOM8A);
		res.push_back(BOM8B);
		res.push_back(BOM8C);
	}
	WC_UTF8((uchar *)&res[int(addBOM)*3],&str[0],str.size());
	return res;
}

std::string UniToSJIS(const std::wstring &str){
	std::string res;
	res.resize(str.size()*sizeof(wchar_t));
	res.resize(WC_SJIS((uchar *)&res[0],&str[0],str.size()));
	return res;
}

bool ConvertSingleCharacter(wchar_t &dst,void *buffer,size_t size,ulong &bytes_used,ENCODING::ENCODING encoding){
	switch (encoding){
		case ENCODING::UTF8:
			return ConvertSingleUTF8Character(dst,(uchar *)buffer,size,bytes_used);
		case ENCODING::SJIS:
			return ConvertSingleSJISCharacter(dst,(uchar *)buffer,size,bytes_used);
		case ENCODING::ISO_8859_1:
			bytes_used=1;
			dst=*(uchar *)buffer;
			return 1;
		default:
			bytes_used=1;
			return 0;
	}
}

ulong find_first_not_of_in_utf8(const std::string &str,const wchar_t *find,ulong start_at){
	while (start_at<str.size()){
		wchar_t c;
		ulong unused;
		if (
				ConvertSingleUTF8Character(c,(const uchar *)&str[start_at],str.size()-start_at,unused) &&
				!multicomparison(c,find))
			return start_at;
		start_at++;
	}
	return ULONG_MAX;
}

ulong find_last_not_of_in_utf8(const std::string &str,const wchar_t *find,ulong start_at){
	if (start_at==ULONG_MAX)
		start_at=str.size()-1;
	while (1){
		wchar_t c;
		ulong unused;
		if (
				ConvertSingleUTF8Character(c,(const uchar *)&str[start_at],str.size()-start_at,unused) &&
				!multicomparison(c,find))
			return start_at;
		if (!start_at)
			break;
		start_at--;
	}
	return ULONG_MAX;
}

bool iswhitespace(wchar_t character){
	const wchar_t *whitespace=WCS_WHITESPACE;
	for (const wchar_t *a=whitespace;*a;a++)
		if (character==*a)
			return 1;
	return 0;
}

bool iswhitespace(char character){
	return (uchar(character)<0x80)?iswhitespaceASCIIe(character):0;
}

bool iswhitespaceASCIIe(char character){
	const char *whitespace=STR_WHITESPACE;
	for (const char *a=whitespace;*a;a++)
		if (character==*a)
			return 1;
	return 0;
}

bool isbreakspace(wchar_t character){
	switch (character){
		case 0x0020:
		case 0x1680:
		case 0x180E:
		case 0x2002:
		case 0x2003:
		case 0x2004:
		case 0x2005:
		case 0x2006:
		case 0x2008:
		case 0x2009:
		case 0x200A:
		case 0x200B:
		case 0x200C:
		case 0x200D:
		case 0x205F:
		case 0x3000:
			return 1;
	}
	return 0;
}

bool isbreakspace(char character){
	return (character>0)?iswhitespaceASCIIe(character):0;
}

bool isbreakspaceASCIIe(char character){
	return character==0x20;
}

bool isValidUTF8(const void *buffer,ulong size){
	const uchar *unsigned_buffer=(const uchar *)buffer;
	for (ulong a=0;a<size;a++){
		ulong char_len;
		if (!(*unsigned_buffer&128))
			char_len=1;
		else if ((*unsigned_buffer&224)==192)
			char_len=2;
		else if ((*unsigned_buffer&240)==224)
			char_len=3;
		else if ((*unsigned_buffer&248)==240)
			char_len=4;
		else
			return 0;
		unsigned_buffer++;
		if (char_len<2)
			continue;
		a++;
		for (ulong b=1;b<char_len;b++,a++,unsigned_buffer++)
			if (*unsigned_buffer<0x80 || (*unsigned_buffer&0xC0)!=0x80)
				return 0;
	}
	return 1;
}

bool isValidSJIS(const void *buffer,ulong size){
	const uchar *unsigned_buffer=(const uchar *)buffer;
	for (ulong a=0;a<size;a++,unsigned_buffer++){
		if (!IS_SJIS_WIDE(*unsigned_buffer)){
			//Don't bother trying to understand what's going on here. It took
			//*me* around ten minutes. It works, and that's all you need to
			//know.
			if (*unsigned_buffer>=0x80 && *unsigned_buffer<=0xA0 || *unsigned_buffer>=0xF0)
				return 0;
			continue;
		}
		a++;
		unsigned_buffer++;
		if (*unsigned_buffer<0x40 || *unsigned_buffer>0xFC || *unsigned_buffer==0x7F)
			return 0;
	}
	return 1;
}

void NONS_tolower(wchar_t *param){
	for (;*param;param++)
		*param=NONS_tolower(*param);
}

void NONS_tolower(char *param){
	for (;*param;param++)
		*param=NONS_tolower(*param);
}

template <typename T>
TiXmlElement *NONS_BasicRect<T>::save(const char *override_name){
	TiXmlElement *rect=new TiXmlElement(override_name?override_name:"rect");
	rect->SetAttribute("x",itoac(this->x));
	rect->SetAttribute("y",itoac(this->y));
	rect->SetAttribute("w",itoac(this->w));
	rect->SetAttribute("h",itoac(this->h));
	return rect;
}
template TiXmlElement *NONS_BasicRect<long>::save(const char *override_name);

template <>
NONS_BasicRect<long>::NONS_BasicRect(TiXmlElement *rect):sdl(0){
	this->x=rect->QueryIntAttribute("x");
	this->y=rect->QueryIntAttribute("y");
	this->w=rect->QueryIntAttribute("w");
	this->h=rect->QueryIntAttribute("h");
}

ErrorCode NT3Decryption(void *vbuffer,size_t length){
    uchar *buffer=(uchar *)vbuffer;
    int key = buffer[0] | 
                  buffer[1] << 8 |
                  buffer[2] << 16 |
                  buffer[3] << 24;
    for (ulong i=4;i<length;i++){
         key=buffer[i]^key;
         key+=0x5D588B65+(buffer[i])*(length-i);
         buffer[i]^=key;
    }
    return NONS_NO_ERROR;
}

