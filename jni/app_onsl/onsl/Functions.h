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

#ifndef NONS_FUNCTIONS_H
#define NONS_FUNCTIONS_H

#include "ErrorCodes.h"
#include "Common.h"
#include "enums.h"
#include "SDLhead.h"
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <ctime>
#include <cassert>

const double pi=3.1415926535897932384626433832795;

#define UNICODE_TOUPPER(x) ((x)&0xDF)
#define UNICODE_TOLOWER(x) ((x)|0x20)

#define ABS(x) ((x)<0?-(x):(x))

#define HANDLE_POSSIBLE_ERRORS(x) {\
	ErrorCode possible_error=(x);\
	if (possible_error!=NONS_NO_ERROR)\
		return possible_error;\
}
#define CHECK_FLAG(flag,pattern) (((flag)&(pattern))==(pattern))
#define ACCURATE_INTEGER_MULTIPLICATION(a,b) (((a)*(b))/255)
#define FAST_INTEGER_MULTIPLICATION(a,b) (((a)*(b))>>8)
#define CHECK_POINTER_AND_CALL(p,c) if (p) (p)->c

#define STR_WHITESPACE "\x09\x0A\x0B\x0C\x0D\x20\x85\xA0"
#define WCS_WHITESPACE L"\x0009\x000A\x000B\x000C\x000D\x0020\x0085\x00A0\x1680\x180E\x2002\x2003\x2004\x2005\x2006\x2008\x2009\x200A\x200B\x200C\x200D\x205F\x3000"
#define WCS_NON_NEWLINE_WHITESPACE L"\x0009\x000B\x000C\x0020\x0085\x00A0\x1680\x180E\x2002\x2003\x2004\x2005\x2006\x2008\x2009\x200A\x200B\x200C\x200D\x205F\x3000"

#define BOM16B 0xFEFF
#define BOM16BA 0xFE
#define BOM16BB 0xFF
#define BOM16L 0xFFFE
#define BOM16LA BOM16BB
#define BOM16LB BOM16BA
#define BOM8A ((uchar)0xEF)
#define BOM8B ((uchar)0xBB)
#define BOM8C ((uchar)0xBF)
#define NONS_BIG_ENDIAN 0
#define NONS_LITTLE_ENDIAN 1
#define UNDEFINED_ENDIANNESS 2

bool iswhitespace(char character);
bool iswhitespace(wchar_t character);
bool iswhitespaceASCIIe(char character);
bool isbreakspace(char character);
bool isbreakspace(wchar_t character);
bool isbreakspaceASCIIe(char character);
void NONS_tolower(wchar_t *param);
void NONS_tolower(char *param);
ErrorCode NT3Decryption(void *vbuffer,size_t length);

inline bool NONS_isdigit(unsigned character){
	return character>='0' && character<='9';
}
inline bool NONS_isupper(unsigned character){
	return character>='A' && character<='Z';
}
inline bool NONS_islower(unsigned character){
	return character>='a' && character<='z';
}
inline bool NONS_isalpha(unsigned character){
	return NONS_isupper(character) || NONS_islower(character);
}
inline bool NONS_isalnum(unsigned character){
	return NONS_isalpha(character) || NONS_isdigit(character);
}
inline unsigned NONS_toupper(unsigned character){
	return NONS_islower(character)?UNICODE_TOUPPER(character):character;
}
inline unsigned NONS_tolower(unsigned character){
	return NONS_isupper(character)?UNICODE_TOLOWER(character):character;
}
inline bool NONS_ishexa(unsigned character){
	return NONS_isdigit(character) || NONS_toupper(character)>='A' && NONS_toupper(character)<='F';
}
//1 if the character matches the regex [A-Za-z_] (the first character in a C-style identifier)
inline bool NONS_isid1char(unsigned character){
	return NONS_isalpha(character) || character=='_';
}
//1 if the character matches the regex [A-Za-z_0-9] (the second and beyond character in a C-style identifier)
inline bool NONS_isidnchar(unsigned character){
	return NONS_isid1char(character) || NONS_isdigit(character);
}

//string functions
template <typename T,typename T2>
bool multicomparison(T character,const T2 *characters){
	for (;*characters;characters++)
		if (character==*characters)
			return 1;
	return 0;
}

template <typename T>
void toforwardslash(std::basic_string<T> &s){
	for (ulong a=0,size=s.size();a<size;a++)
		s[a]=(s[a]==0x5C)?0x2F:s[a];
}

template <typename T>
void tobackslash(std::basic_string<T> &s){
	for (ulong a=0,size=s.size();a<size;a++)
		s[a]=(s[a]==0x2F)?0x5C:s[a];
}

template <typename T>
std::vector<std::basic_string<T> > getParameterList(const std::basic_string<T> &string,bool leave_quotes,char delim=' '){
	std::vector<std::basic_string<T> > res;
	char tempDelim=delim;
	for (ulong a=0,size=string.size();a<size;){
		if (string[a]=='\"'){
			if (!leave_quotes)
				a++;
			delim='\"';
		}
		ulong end=a;
		if (delim!=tempDelim && leave_quotes)
			end++;
		for (;end<size && string[end]!=delim;end++);
		if (delim!=tempDelim && leave_quotes)
			end++;
		res.push_back(std::basic_string<T>(string,a,end-a));
		delim=tempDelim;
		for (a=end+1;a<size && string[a]==delim;a++);
	}
	return res;
}

/*bool filenames_are_equal(const wchar_t *str0,const wchar_t *str1);
bool filenames_are_equal(const char *str0,const char *str1);*/
template <typename T>
bool isValidIdentifier(const std::basic_string<T> &str){
	if (!str.size() || !NONS_isid1char(str[0]))
		return 0;
	const T *s=&str[1];
	for (ulong a=1,size=str.size();a<size;a++,s++)
		if (!NONS_isidnchar(*s))
			return 0;
	return 1;
}

template <typename T>
bool isValidLabel(const std::basic_string<T> &str){
	const T *s=&str[0];
	for (ulong a=0,size=str.size();a<size;a++,s++)
		if (!NONS_isidnchar(*s))
			return 0;
	return 1;
}

template <typename T>
inline T HEX2DEC(T x){
	return x<='9'?x-'0':(x<='F'?x-'A'+10:x-'a'+10);
}

template <typename T,typename T2>
std::basic_string<T> itoa(T2 n,unsigned w=0){
#ifndef PSP
	std::basic_stringstream<T> stream;
	if (w){
		stream.fill('0');
		stream.width(w);
	}
	stream <<n;
	return stream.str();
#else
	bool sign=n<0;
	if (n<0)
		n=-n;
	std::basic_string<T> res;
	while (n>0){
		res.push_back(n%10+'0');
		n/=10;
	}
	while (res.size()<1 || res.size()<w)
		res.push_back('0');
	if (sign)
		res[res.size()-1]='-';
	std::reverse(res.begin(),res.end());
	return res;
#endif
}
template <typename T> inline std::string  itoac(T n,unsigned w=0){ return itoa<char>   (n,w); }
template <typename T> inline std::wstring itoaw(T n,unsigned w=0){ return itoa<wchar_t>(n,w); }

template <typename T,typename T2>
std::basic_string<T> itohex(T2 n,unsigned w=0){
	std::basic_stringstream<T> stream;
	if (w){
		stream.fill('0');
		stream.width(w);
	}
	stream <<std::hex<<n;
	return stream.str();
}
template <typename T> inline std::string  itohexc(T n,unsigned w=0){ return itohex<char>   (n,w); }
template <typename T> inline std::wstring itohexw(T n,unsigned w=0){ return itohex<wchar_t>(n,w); }

template <typename T>
bool begins_with(const T *s1,const T *s2){
	for (;*s2;s1++,s2++)
		if (*s1!=*s2)
			return 0;
	return 1;
}

template <typename T>
std::basic_string<T> normalize_path(std::basic_string<T> path){
	toforwardslash(path);
	bool r;
	do{
		r=0;
		std::basic_string<T> temp;
		for (size_t a=0;a<path.size();){
			T c;
			static T string_0[]={'/','/',0};
			static T string_1[]={'/','.','/',0};
			static T string_2[]={'.','.','/',0};
			if (begins_with(&path[a],string_0)){
				c='/';
				r=1;
				a+=2;
			}else if (begins_with(&path[a],string_1)){
				c='/';
				r=1;
				a+=3;
			}else if (begins_with(&path[a],string_2)){
				c=0;
				if (temp.size()){
					temp.erase(temp.end()-1);
					while (temp.size() && temp[temp.size()-1]!='/')
						temp.erase(temp.end()-1);
				}
				r=1;
				a+=3;
			}else
				c=path[a++];
			if (!c)
				continue;
			temp.push_back(c);
		}
		path=temp;
	}while (r);
	return path;
}

//1 if the s1 begins with s2 at off
template <typename T>
bool begins_with(const std::basic_string<T> &s1,size_t off,const std::basic_string<T> &s2){
	if (s1.size()-off<s2.size())
		return 0;
	return begins_with(&s1[off],&s2[0]);
}

template <typename T>
bool begins_with(const std::basic_string<T> &s1,size_t off,const T *s2){
	ulong l=0;
	while (s2[l])
		l++;
	if (s1.size()-off<l)
		return 0;
	return begins_with(&s1[off],s2);
}

template <typename T>
inline bool ends_with(const std::basic_string<T> &s1,const std::basic_string<T> &s2){
	return s1.size()>=s2.size() && begins_with(s1,s1.size()-s2.size(),s2);
}

template <typename T>
std::basic_string<T> string_replace(
		const std::basic_string<T> &src,
		const std::basic_string<T> &what,
		const std::basic_string<T> &with){
	if (!what.size())
		return src;
	std::basic_string<T> res;
	for (ulong a=0,end=src.size();a<end;){
		ulong found=src.find(what,a);
		if (found!=src.npos){
			res.append(src,a,found-a);
			res.append(with);
			a=found+what.size();
		}else{
			res.append(src,a,found);
			a=found;
		}
	}
	return res;
}

template <typename T>
inline std::basic_string<T> string_replace(
		const std::basic_string<T> &src,
		const T *what,
		const T *with){
	if (!what || !*what)
		return src;
	std::basic_string<T> res;
	ulong l=0,l2=0;
	while (what[l])
		l++;
	if (with)
		while (with[l2])
			l2++;
	for (ulong a=0,end=src.size();a<end;){
		ulong found=src.find(what,a,l);
		if (found!=src.npos){
			res.append(src,a,found-a);
			if (with)
				res.append(with,l2);
			a=found+l;
		}else{
			res.append(src,a,found);
			a=found;
		}
	}
	return res;
}


template <typename T>
void trim_string(std::basic_string<T> &str){
	ulong first=0,
		second,
		size=str.size();
	for (;first<size && iswhitespace(str[first]);first++);
	if (first==size){
		str.clear();
		return;
	}
	second=size-1;
	for (;second>first && iswhitespace(str[second]);second--);
	second++;
	str=str.substr(first,second-first);
}

#undef min
//string parsing
template <typename T>
std::basic_string<T> tag_name(const std::basic_string<T> &string,size_t off){
	if (string[off]!='<')
		return std::basic_string<T>();
	ulong a=string.find('>',off+1),
		b=string.find('=',off+1);
	a=std::min(a,b);
	if (a==string.npos)
		return std::basic_string<T>();
	std::basic_string<T> temp(string,off+1,a-(off+1));
	trim_string(temp);
	return temp;
}

template <typename T>
std::basic_string<T> tag_value(const std::basic_string<T> &string,size_t off){
	if (string[off]!='<')
		return std::basic_string<T>();
	ulong a=string.find('>',off+1),
		b=string.find('=',off+1);
	a=std::min(a,b);
	if (a==string.npos || string[a]=='>')
		return std::basic_string<T>();
	a++;
	ulong c=string.find('>',a);
	std::basic_string<T> temp(string,a,a-c);
	trim_string(temp);
	return temp;
}

template <typename T>
std::basic_string<T> remove_tags(const std::basic_string<T> &str){
	std::basic_string<T> res;
	res.reserve(str.size());
	for (ulong a=0;a<str.size();a++){
		switch (str[a]){
			case '<':
				while (str[a]!='>')
					a++;
				break;
			case '\\':
				a++;
			default:
				res.push_back(str[a]);
				break;
		}
	}
	return res;
}

//binary parsing functions
bool getbit(void *arr,ulong *byteoffset,uchar *bitoffset);
ulong getbits(void *arr,uchar bits,ulong *byteoffset,uchar *bitoffset);
Uint8 readByte(const void *buffer,ulong &offset);
Sint16 readSignedWord(const void *buffer,ulong &offset);
Uint16 readWord(const void *buffer,ulong &offset);
Sint32 readSignedDWord(const void *buffer,ulong &offset);
Uint32 readDWord(const void *buffer,ulong &offset);
Uint64 readQWord(const void *buffer,ulong &offset);
std::string readString(const void *buffer,ulong &offset);
void writeByte(Uint8 a,std::vector<uchar> &str,ulong offset=ULONG_MAX);
void writeWord(Uint16 a,std::vector<uchar> &str,ulong offset=ULONG_MAX);
void writeDWord(Uint32 a,std::vector<uchar> &str,ulong offset=ULONG_MAX);
void writeWordBig(Uint16 a,std::vector<uchar> &str,ulong offset=ULONG_MAX);
void writeDWordBig(Uint32 a,std::vector<uchar> &str,ulong offset=ULONG_MAX);
void writeString(const std::wstring &a,std::vector<uchar> &str);

template <typename T>
std::vector<Sint32> getIntervals(typename std::map<Sint32,T>::iterator i,typename std::map<Sint32,T>::iterator end){
	std::vector<Sint32> intervals;
	Sint32 last=i->first;
	intervals.push_back(last++);
	while (++i!=end){
		if (i->first!=last){
			intervals.push_back(last-intervals.back());
			last=i->first;
			intervals.push_back(last++);
		}else
			last++;
	}
	intervals.push_back(last-intervals.back());
	return intervals;
}

//other functions
Uint32 secondsSince1970();
/*
Compresses src[0..srcl-1].
Return value: allocated compressed buffer.
src: input buffer.
srcl: length of the input buffer in bytes.
dstl: the length of the compressed buffer will be written here.
*/
uchar *compressBuffer_BZ2(uchar *src,size_t src_l,size_t &dstl);
/*
Decompresses src[0..srcl-1].
Return value: allocated decompressed buffer.
src: input buffer.
srcl: length of the input buffer in bytes.
dstl: the length of the decompressed buffer will be written here.
*/
uchar *decompressBuffer_BZ2(uchar *src,size_t src_l,size_t &dst_l);

template <typename T1,typename T2>
bool binary_search(const T1 *set,size_t begin,size_t end,const T2 &value,size_t &at_offset,int (*comp_f)(const T2 &,const T1 &)){
	if (begin<=end){
		size_t size=end-begin+1;
		while (begin<=end){
			size_t pivot=begin+size/2;
			int cmp=comp_f(value,set[pivot]);
			if (size==1){
				if (!cmp){
					at_offset=pivot;
					return 1;
				}
				break;
			}
			if (cmp<0)
				end=pivot-1;
			else if (cmp>0)
				begin=pivot+1;
			else{
				at_offset=pivot;
				return 1;
			}
			size=end-begin+1;
		}
	}
	return 0;
}

ErrorCode inPlaceDecryption(void *buffer,size_t length,ulong mode);

#if NONS_SYS_WINDOWS
void findMainWindow(const wchar_t *caption);
#endif

//Unicode functions:
template <typename T2,typename T>
T2 ato(const std::basic_string<T> &str){
#if !NONS_SYS_PSP
	std::basic_stringstream<T> stream(str);
	T2 res;
	return !(stream >>res)?0:res;
#else
	T2 res=0;
	bool sign=0;
	ulong a=0;
	if (str[a]=='-'){
		sign=1;
		a++;
	}
	for (;a<str.size() && NONS_isdigit(str[a]);a++)
		res=res*10+str[a]-'0';
	if (sign)
		res=-res;
	return res;
#endif
}

template <typename T> inline long atol(const std::basic_string<T> &str){ return ato<long>(str); }
template <typename T> inline ulong atoul(const std::basic_string<T> &str){ return ato<ulong>(str); }
template <typename T> inline double atof(const std::basic_string<T> &str){ return ato<double>(str); }

template <typename T1,typename T2>
int lexcmp(const T1 *a,const T2 *b){
	for (;*a || *b;a++,b++){
		unsigned c=*a,
			d=*b;
		if (c<d)
			return -1;
		if (c>d)
			return 1;
	}
	return 0;
}

template <typename T1,typename T2>
int lexcmp_CI(const T1 *a,const T2 *b){
	for (;*a || *b;a++,b++){
		unsigned c=NONS_toupper(*a),
			d=NONS_toupper(*b);
		if (c<d)
			return -1;
		if (c>d)
			return 1;
	}
	return 0;
}

template <typename T1,typename T2>
int lexcmp_CI_bounded(const T1 *a,size_t sizeA,const T2 *b,size_t sizeB){
	for (size_t c=0;c<sizeA && c<sizeB;a++,b++,c++){
		unsigned d=NONS_toupper(*a),
			e=NONS_toupper(*b);
		if (d<e)
			return -1;
		if (d>e)
			return 1;

	}
	if (sizeA<sizeB)
		return -1;
	if (sizeA>sizeB)
		return 1;
	return 0;
}

ulong getUTF8size(const wchar_t *buffer,ulong size);

template <typename T>
std::wstring UniFromISO88591(const T &str){
	std::wstring res;
	res.resize(str.size());
	void ISO_WC(wchar_t *dst,const uchar *src,ulong srcl);
	ISO_WC(&res[0],(const uchar *)&str[0],str.size());
	return res;
}

template <typename T>
std::wstring UniFromUTF8(const T &str){
	ulong start=0;
	if (str.size()>=3 && (uchar)str[0]==BOM8A && (uchar)str[1]==BOM8B && (uchar)str[2]==BOM8C)
		start+=3;
	const uchar *str2=(const uchar *)&str[0]+start;
	ulong size=0;
	for (ulong a=start,end=str.size();a<end;a++,str2++)
		if (*str2<128 || (*str2&192)==192)
			size++;
	std::wstring res;
	res.resize(size);
	str2=(const uchar *)&str[0]+start;
	void UTF8_WC(wchar_t *dst,const uchar *src,ulong srcl);
	UTF8_WC(&res[0],str2,str.size()-start);
	return res;
}

template <typename T>
std::wstring UniFromSJIS(const T &str){
	std::wstring res;
	res.resize(str.size());
	ulong SJIS_WC(wchar_t *dst,const uchar *src,ulong srcl);
	res.resize(SJIS_WC(&res[0],(const uchar *)&str[0],str.size()));
	return res;
}

std::string UniToISO88591(const std::wstring &str);
std::string UniToUTF8(const std::wstring &str,bool addBOM=0);
std::string UniToSJIS(const std::wstring &str);
bool ConvertSingleCharacter(wchar_t &dst,void *buffer,size_t size,ulong &bytes_used,ENCODING::ENCODING encoding);
void ConvertSingleCharacterToUTF8(char dst[4],wchar_t src,ulong &bytes_used);
ulong find_first_not_of_in_utf8(const std::string &,const wchar_t *,ulong =0);
ulong find_last_not_of_in_utf8(const std::string &,const wchar_t *,ulong =ULONG_MAX);

template <typename T>
inline void toupper(std::basic_string<T> &str){
	std::transform<
		typename std::basic_string<T>::iterator,
		typename std::basic_string<T>::iterator,
		unsigned(*)(unsigned)>(str.begin(),str.end(),str.begin(),NONS_toupper);
}

template <typename T>
inline void tolower(std::basic_string<T> &str){
	std::transform<
		typename std::basic_string<T>::iterator,
		typename std::basic_string<T>::iterator,
		unsigned(*)(unsigned)>(str.begin(),str.end(),str.begin(),NONS_tolower);
}

template <typename T>
inline std::basic_string<T> toupperCopy(std::basic_string<T> str){
	toupper(str);
	return str;
}

template <typename T>
inline std::basic_string<T> tolowerCopy(std::basic_string<T> str){
	tolower(str);
	return str;
}

inline std::ostream &operator<<(std::ostream &stream,std::wstring &str){
	return stream <<UniToUTF8(str);
}

char checkEnd(wchar_t a);
//Determines the system's endianness.
char checkNativeEndianness();

bool isValidUTF8(const void *buffer,ulong size);
bool isValidSJIS(const void *buffer,ulong size);

template <typename T>
struct stdStringCmpCI{
	bool operator()(const std::basic_string<T> &s1,const std::basic_string<T> &s2) const{
		return lexcmp_CI_bounded(s1.c_str(),s1.size(),s2.c_str(),s2.size())<0;
	}
};

template <typename T1,typename T2>
int stdStrCmpCI(const std::basic_string<T1> &s1,const T2 *s2){
	std::basic_string<T2> temp=s2;
	return lexcmp_CI_bounded(s1.c_str(),s1.size(),temp.c_str(),temp.size());
}

template <typename T1,typename T2>
int stdStrCmpCI(const std::basic_string<T1> &s1,const std::basic_string<T2> &s2){
	return lexcmp_CI_bounded(s1.c_str(),s1.size(),s2.c_str(),s2.size());
}

template <typename T>
bool firstcharsCI(const T *s1,const T *s2){
	for (;*s2;s1++,s2++)
		if (NONS_tolower(*s1)!=NONS_tolower(*s2))
			return 0;
	return 1;
}

//1 if the s1 begins with s2 at off
template <typename T>
bool firstcharsCI(const std::basic_string<T> &s1,size_t off,const std::basic_string<T> &s2){
	if (s1.size()-off<s2.size())
		return 0;
	return firstcharsCI(&s1[off],&s2[off]);
}

template <typename T>
bool firstcharsCI(const std::basic_string<T> &s1,size_t off,const T *s2){
	ulong l=0;
	while (s2[l])
		l++;
	if (s1.size()-off<l)
		return 0;
	return firstcharsCI(&s1[off],s2);
}

template <typename T>
std::basic_string<T> getTimeString(tm *time,bool useISO=0){
	const char *strings[]={
		"%Y-%m-%d %H:%M:%S",
		"%Y%m%dT%H%M%S"
	};
	char buffer[]="0000-00-00 00:00:00";
	strftime(buffer,sizeof(buffer),strings[(int)useISO],time);
	std::string temp=buffer;
	return std::basic_string<T>(temp.begin(),temp.end());
}

template <typename T>
std::basic_string<T> getTimeString(bool useISO=0,bool useLocalTime=1){
	typedef tm *(*time_f)(const time_t *);
	time_f functions[]={
		gmtime,
		localtime
	};
	time_t secs=time(0);
	return getTimeString<T>(functions[(int)useLocalTime](&secs),useISO);
}

struct surfaceData{
	uchar *pixels;
	uchar Roffset,
		Goffset,
		Boffset,
		Aoffset;
	uchar offsets[4];
	ulong advance,
		pitch,
		w,h;
	bool alpha;
};

class TiXmlElement;

template <typename T>
struct NONS_BasicRect{
	T x,y,w,h;
	SDL_Rect *sdl;
	NONS_BasicRect<T>(T x=0,T y=0,T w=0,T h=0):x(x),y(y),w(w),h(h),sdl(0){}
	NONS_BasicRect<T>(const NONS_BasicRect<T> &s):x(s.x),y(s.y),w(s.w),h(s.h),sdl(0){}
	NONS_BasicRect<T>(TiXmlElement *);
	template <typename T2>
	explicit NONS_BasicRect<T>(const T2 &s):x((T)s.x),y((T)s.y),w((T)s.w),h((T)s.h),sdl(0){}
	~NONS_BasicRect(){ delete this->sdl; }
	SDL_Rect &to_SDL_Rect(){
		if (!this->sdl)
			this->sdl=new SDL_Rect;
		this->sdl->x=(Sint16)this->x;
		this->sdl->y=(Sint16)this->y;
		this->sdl->w=(Uint16)this->w;
		this->sdl->h=(Uint16)this->h;
		return *(this->sdl);
	}
	template <typename T2>
	NONS_BasicRect<T> &operator=(const T2 &s){
		this->x=(T)s.x;
		this->y=(T)s.y;
		this->w=(T)s.w;
		this->h=(T)s.h;
		return *this;
	}
	NONS_BasicRect<T> &operator=(const NONS_BasicRect<T> &s){
		return this->operator=<NONS_BasicRect<T> >(s);
	}
	NONS_BasicRect<T> intersect(const NONS_BasicRect<T> &b){
		NONS_BasicRect<T> r(
			std::max(this->x,b.x),
			std::max(this->y,b.y),
			std::min(this->x+this->w,b.x+b.w),
			std::min(this->y+this->h,b.y+b.h)
		);
		r.w-=r.x;
		r.h-=r.y;
		return r;
	}
	bool point_is_inside(T x,T y){
		return (x>=this->x && y>=this->y && x<this->x+this->w && y<this->y+this->h);
	}
	bool point_is_inside(const NONS_BasicRect<T> &b){
		return (b.x>=this->x && b.y>=this->y && b.x<this->x+this->w && b.y<this->y+this->h);
	}
	TiXmlElement *save(const char *override_name=0);
};
typedef NONS_BasicRect<float> NONS_Rect;
typedef NONS_BasicRect<long> NONS_LongRect;

template <typename T>
void freePointerVector(std::vector<T *> &v){
	for (size_t a=0;a<v.size();a++)
		if (v[a])
			delete v[a];
	v.clear();
}

template <typename T>
std::basic_string<T> generate_filename(){
	std::basic_string<T> r=getTimeString<T>(1);
	r.push_back('_');
	r.append(itoa<T>(SDL_GetTicks(),10));
	return r;
}

template <typename dst_t,typename src_t>
void append(dst_t &dst,const src_t &src,size_t start_from=0){
	if (start_from>=src.size())
		return;
	size_t n=dst.size();
	dst.resize(n+src.size()-start_from);
	std::copy(src.begin()+start_from,src.end(),dst.begin()+n);
}

template <typename dst_t,typename src_t>
void copy_container(dst_t &dst,const src_t &src){
	dst.resize(src.size());
	std::copy(src.begin(),src.end(),dst.begin());
}

template <typename Iterator,typename Container>
Iterator advance_iterator(Iterator i,int dir,Container &m){
	Iterator b=m.begin(),
		e=m.end();
	if (dir<0){
		if (i==b)
			i=e;
		--i;
	}else{
		if (i==e)
			i=b;
		else{
			++i;
			if (i==e)
				i=b;
		}
	}
	return i;

}

template <typename T>
void saturate_value(T &dst,const T &min,const T &max){
	if (dst<min)
		dst=min;
	else if (dst>max)
		dst=max;
}

class TiXmlNode;
TiXmlNode *find_xml_node(TiXmlNode *node,const std::wstring &name);

template <typename T>
void normalize_line_endings(std::basic_string<T> &s){
	T *p=&s[0];
	size_t write=0;
	for (size_t read=0,n=s.size();read<n;read++,write++){
		if (p[read]==13){
			if (read+1<n && p[read+1]==10)
				read++;
			else{
				p[write]=10;
				continue;
			}
		}
		p[write]=p[read];
	}
	s.resize(write);
}

template <typename T>
size_t find_last_not_of_id(const std::basic_string<T> &str,size_t pos=std::basic_string<T>::npos){
	if (!str.size())
		return str.npos;
	if (pos==str.npos || pos>=str.size())
		pos=str.size()-1;
	const T *p=&str[0];
	while (1){
		if (!NONS_isidnchar(p[pos]))
			break;
		if (!pos--)
			return str.npos;
	}
	return pos;
}
#endif
