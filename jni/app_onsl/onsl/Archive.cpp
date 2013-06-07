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

#include "Archive.h"
#include "Options.h"
#include "LZMA.h"
#include <bzlib.h>
#include <iostream>
#include <cassert>

template <typename T> inline void zero_structure(T &s){ memset(&s,0,sizeof(s)); }

NONS_GeneralArchive general_archive;

Uint32 CRC32::CRC32lookup[]={
	0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,
	0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
	0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,
	0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
	0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,
	0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
	0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
	0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
	0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,
	0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
	0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,
	0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
	0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,
	0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
	0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
	0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
	0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,
	0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
	0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,
	0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
	0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
	0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
	0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,
	0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
	0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,
	0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
	0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,
	0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
	0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,
	0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
	0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,
	0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
};



template <typename T>
inline size_t find_slash(const std::basic_string<T> &str,size_t off=0){
	const T *p=&str[0];
	size_t n=str.size();
	for (size_t a=off;a<n;a++)
		if (p[a]=='/' || p[a]=='\\')
			return a;
	return str.npos;
}

//------------------------------------------------------------------------------
//------------------- COMPRESSION-RELATED FUNCTIONS ----------------------------
//------------------------------------------------------------------------------

unsigned decompress_from_regular_file::in_f(void *p,unsigned char **buffer){
	decompress_from_regular_file *_this=(decompress_from_regular_file *)p;
	size_t l=_this->default_in_size;
	_this->in.resize(l);
	_this->file->read(&_this->in[0],l,l,_this->offset);
	_this->in.resize(l);
	if (!l){
		_this->in_buffer=0;
		if (buffer)
			*buffer=0;
		_this->remaining=0;
		return 0;
	}
	_this->in_buffer=&_this->in[0];
	if (buffer)
		*buffer=_this->in_buffer;
	_this->remaining=l;
	_this->offset+=l;
	return _this->in.size();
}

unsigned decompress_from_file::in_f(void *p,unsigned char **buffer){
	decompress_from_file *_this=(decompress_from_file *)p;
	size_t l=_this->default_in_size;
	_this->in.resize(l);
	_this->archive->read_raw_bytes(&_this->in[0],l,l,_this->node,_this->offset);
	_this->in.resize(l);
	if (!l){
		_this->in_buffer=0;
		if (buffer)
			*buffer=0;
		_this->remaining=0;
		return 0;
	}
	_this->in_buffer=&_this->in[0];
	if (buffer)
		*buffer=_this->in_buffer;
	_this->remaining=l;
	_this->offset+=l;
	return _this->in.size();
}

unsigned decompress_from_memory::in_f(void *p,unsigned char **buffer){
	decompress_from_memory *_this=(decompress_from_memory *)p;
	if (buffer)
		*buffer=_this->src;
	size_t a=_this->remaining;
	_this->src+=a;
	_this->remaining=0;
	return a;
}

int decompress_to_file::out_f(void *p,unsigned char *buffer,unsigned size){
	Uint64 &limit=((decompress_to_file *)p)->output_limit;
	assert(limit!=0);
	if (size>limit)
		size=(unsigned)limit;
	((decompress_to_file *)p)->file->write(buffer,size);
	((decompress_to_file *)p)->crc32.Input(buffer,size);
	limit-=size;
	return 0;
}

int decompress_to_memory::out_f(void *p,unsigned char *buffer,unsigned size){
	decompress_to_memory *_this=(decompress_to_memory *)p;
	Uint64 &limit=_this->output_limit;
	if (size>limit)
		size=(unsigned)limit;
	memcpy(_this->buffer,buffer,size);
	limit-=size;
	_this->buffer=(uchar *)_this->buffer+size;
	return 0;
}

int decompress_to_unknown_size::out_f(void *p,unsigned char *buffer,unsigned size){
	decompress_to_unknown_size *_this=(decompress_to_unknown_size *)p;
	_this->dst->push_back(std::vector<uchar>(buffer,buffer+size));
	_this->final_size+=size;
	return 0;
}

typedef bool (*decompression_f)(base_out_decompression *,base_in_decompression *);

bool DecompressDEFLATE(base_out_decompression *dst,base_in_decompression *src){
	z_stream stream;
	zero_structure(stream);

	if (inflateBackInit(&stream,dst->bits,&dst->out[0])!=Z_OK)
		return 0;
	int res=inflateBack(
		&stream,
		src->get_f(),
		src,
		dst->get_f(),
		dst
	);
	inflateBackEnd(&stream);
	return res==Z_STREAM_END;
}

bool DecompressBZ2(base_out_decompression *dst,base_in_decompression *src){
	bz_stream stream;
	zero_structure(stream);

	stream.next_in=0;
	stream.avail_in=0;
	stream.next_out=(char *)&dst->out[0];
	stream.avail_out=dst->size;
	if (BZ2_bzDecompressInit(&stream,0,0)!=BZ_OK)
		return 0;
	int res;
	in_func in=src->get_f();
	out_func out=dst->get_f();
	while ((res=BZ2_bzDecompress(&stream))==BZ_OK){
		if (!stream.avail_out){
			out(dst,&dst->out[0],dst->size);
			stream.next_out=(char *)&dst->out[0];
			stream.avail_out=dst->size;
		}
		if (!stream.avail_in && !(stream.avail_in=in(src,(uchar **)&stream.next_in))){
			while (1){
				BZ2_bzDecompress(&stream);
				if (stream.avail_out<dst->size){
					out(dst,&dst->out[0],dst->size);
					stream.next_out=(char *)&dst->out[0];
					stream.avail_out=dst->size;
				}else
					break;
			}
			break;
		}
	}
	bool ret=1;
	if (res!=BZ_STREAM_END)
		ret=0;
	else if (stream.avail_out<dst->size)
		out(dst,&dst->out[0],dst->size-stream.avail_out);
	BZ2_bzDecompressEnd(&stream);
	return ret;
}

bool CompressBZ2(base_out_decompression *dst,base_in_decompression *src){
	bz_stream stream;
	zero_structure(stream);

	in_func in=src->get_f();
	out_func out=dst->get_f();
	stream.avail_in=in(src,(uchar **)&stream.next_in);
	stream.next_out=(char *)&dst->out[0];
	stream.avail_out=dst->size;
	if (BZ2_bzCompressInit(&stream,1,0,0)!=BZ_OK)
		return 0;
	int res;
	int action=BZ_RUN;
	while (1){
		res=BZ2_bzCompress(&stream,action);
		if (res!=BZ_OK && res!=BZ_RUN_OK && res!=BZ_FINISH_OK)
			break;
		if (!stream.avail_out){
			out(dst,&dst->out[0],dst->size);
			stream.next_out=(char *)&dst->out[0];
			stream.avail_out=dst->size;
		}
		if (!stream.avail_in)
			if (!(stream.avail_in=in(src,(uchar **)&stream.next_in)))
				action=BZ_FINISH;
	}
	bool ret=1;
	if (res!=BZ_STREAM_END)
		ret=0;
	else if (stream.avail_out<dst->size)
		out(dst,&dst->out[0],dst->size-stream.avail_out);
	BZ2_bzCompressEnd(&stream);
	return ret;
}

static void *SzAlloc(void *p, size_t size) { return malloc(size); }
static void SzFree(void *p, void *address) { free(address); }

bool DecompressLZMA(base_out_decompression *dst,base_in_decompression *src){
	bz_stream stream;
	zero_structure(stream);

	in_func in=src->get_f();
	out_func out=dst->get_f();
	std::vector<uchar> dictionary;
	in(src,0);
	if (src->remaining<4+LZMA_PROPS_SIZE)
		return 0;
	CLzmaDec p;
	LzmaDec_Construct(&p);
	ISzAlloc alloc={SzAlloc,SzFree};
	SRes res=LzmaDec_AllocateProbs(&p,(const Byte *)src->in_buffer+4,LZMA_PROPS_SIZE,&alloc);
	if (res)
		return 0;
	dictionary.resize(p.prop.dicSize);
	p.dic=(Byte *)&dictionary[0];
	p.dicBufSize=dictionary.size();
	LzmaDec_Init(&p);
	src->in_buffer+=4+LZMA_PROPS_SIZE;
	src->remaining-=4+LZMA_PROPS_SIZE;
	SizeT srcLen=src->remaining,
		dstLen=dst->size;
	ELzmaStatus status;
	size_t in_pos=0,
		out_pos=0;
	ELzmaFinishMode finish=LZMA_FINISH_ANY;
	while (1){
		res=LzmaDec_DecodeToBuf(
			&p,
			(Byte *)&dst->out[out_pos],
			&dstLen,
			(const Byte *)src->in_buffer+in_pos,
			&srcLen,
			finish,
			&status
		);
		if (res!=SZ_OK || status==LZMA_STATUS_FINISHED_WITH_MARK)
			break;
		out_pos+=dstLen;
		dstLen=dst->out.size()-out_pos;
		if (!dstLen){
			out(dst,&dst->out[0],dst->size);
			dstLen=dst->size;
			out_pos=0;
		}
		in_pos+=srcLen;
		srcLen=src->remaining-in_pos;
		if (!srcLen){
			srcLen=in(src,0);
			if (!srcLen){
				if (finish!=LZMA_FINISH_END)
					finish=LZMA_FINISH_END;
				else
					break;
			}
			in_pos=0;
		}
	}
	bool ret=1;
	size_t write_size=0;
	if (status!=LZMA_STATUS_FINISHED_WITH_MARK){
		if (status==LZMA_STATUS_NEEDS_MORE_INPUT){
			if (dstLen<dst->size)
				write_size=dst->size-dstLen;
		}else
			ret=0;
	}else if (out_pos+dstLen)
		write_size=out_pos+dstLen;
	if (write_size)
		out(dst,&dst->out[0],write_size);
	LzmaDec_FreeProbs(&p,&alloc);
	return ret;
}

uchar *decode_LZSS(uchar *buffer,ulong compressedSize,ulong decompressedSize){
	uchar decompression_buffer[256*2];
	ulong decompresssion_buffer_offset=239;
	memset(decompression_buffer,0,239);
	uchar *res=new uchar[decompressedSize];
	ulong byteoffset=0;
	uchar bitoffset=0;
	for (ulong len=0;len<decompressedSize;){
		if (getbit(buffer,&byteoffset,&bitoffset)){
			uchar a=(uchar)getbits(buffer,8,&byteoffset,&bitoffset);
			res[len++]=a;
			decompression_buffer[decompresssion_buffer_offset++]=a;
			decompresssion_buffer_offset&=0xFF;
		}else{
			uchar a=(uchar)getbits(buffer,8,&byteoffset,&bitoffset);
			uchar b=(uchar)getbits(buffer,4,&byteoffset,&bitoffset);
			for (long c=0;c<=b+1 && len<decompressedSize;c++){
				uchar d=decompression_buffer[(a+c)&0xFF];
				res[len++]=d;
				decompression_buffer[decompresssion_buffer_offset++]=d;
				decompresssion_buffer_offset&=0xFF;
			}
		}
	}
	return res;
}

template <typename T1>
bool decompress_file_to_file(
		const std::wstring &path,
		Archive *a,
		TreeNode *n,
		const T1 &data,
		const std::wstring &name,
		Uint32 &crc32,
		Uint64 offset=0){
	decompression_f f=select_function(data.compression);
	if (!f){
		o_stderr <<"Unsupported compression method used for "<<name<<".\n";
		return 0;
	}
	NONS_File file(path,0);
	decompress_from_file dff;
	decompress_to_file dtf;
	dff.archive=a;
	dff.node=n;
	dff.offset=offset;
	dtf.file=&file;
	dtf.compute_crc=!!crc32;
	dtf.output_limit=data.uncompressed;
	if (!f(&dtf,&dff)){
		o_stderr <<"File "<<name<<" failed to decompress for some reason.\n";
		file.close();
		NONS_File::delete_file(path);
		return 0;
	}
	if (crc32)
		crc32=dtf.crc32.Result();
	return 1;
}

template <typename T>
bool decompress_file_to_memory(void *dst,Archive *a,TreeNode *n,const T &data,const std::wstring &name,Uint64 offset=0){
	decompression_f f=select_function(data.compression);
	if (!f){
		o_stderr <<"Unsupported compression method used for "<<name<<".\n";
		return 0;
	}
	decompress_from_file dff;
	decompress_to_memory dtm;
	dff.archive=a;
	dff.node=n;
	dff.offset=offset;
	dtm.buffer=dst;
	dtm.output_limit=data.uncompressed;
	if (!f(&dtm,&dff)){
		o_stderr <<"File "<<name<<" failed to decompress for some reason.\n";
		return 0;
	}
	return 1;
}

template <typename T>
uchar *decompress_memory_to_new_memory(size_t &dst_l,void *src,size_t src_l,T compression){
	decompression_f f=select_function(compression);
	if (!f)
		return 0;
	decompress_from_memory dfm;
	dfm.remaining=src_l;
	dfm.src=(uchar *)src;
	decompress_to_unknown_size dtus;
	std::list<std::vector<uchar> > temp;
	dtus.dst=&temp;
	//turn on all bits (no output limit)
	dtus.output_limit^=~dtus.output_limit;
	if (!f(&dtus,&dfm))
		return 0;
	uchar *ret=new uchar[dtus.final_size];
	for (ulong a=0;a<dtus.final_size;){
		size_t b=temp.front().size();
		if (b)
			memcpy(ret+a,&(temp.front())[0],b);
		a+=b;
		temp.pop_front();
	}
	dst_l=dtus.final_size;
	return ret;
}

uchar *compress_memory_to_new_memory(size_t &dst_l,void *src,size_t src_l){
	decompress_from_memory dfm;
	dfm.remaining=src_l;
	dfm.src=(uchar *)src;
	decompress_to_unknown_size dtus;
	std::list<std::vector<uchar> > temp;
	dtus.dst=&temp;
	if (!CompressBZ2(&dtus,&dfm))
		return 0;
	uchar *ret=new uchar[dtus.final_size];
	for (ulong a=0;a<dtus.final_size;){
		size_t b=temp.front().size();
		if (b)
			memcpy(ret+a,&(temp.front())[0],b);
		a+=b;
		temp.pop_front();
	}
	dst_l=dtus.final_size;
	return ret;
}

uchar *compressBuffer_BZ2(uchar *src,size_t src_l,size_t &dst_l){
	return compress_memory_to_new_memory(dst_l,src,src_l);
}

uchar *decompressBuffer_BZ2(uchar *src,size_t src_l,size_t &dst_l){
	return decompress_memory_to_new_memory(dst_l,src,src_l,ZIPdata::COMPRESSION_BZ2);
}

//------------------------------------------------------------------------------
//------------------- ~COMPRESSION-RELATED FUNCTIONS ---------------------------
//------------------------------------------------------------------------------

TreeNode::~TreeNode(){
	if (this->freeExtraData)
		this->freeExtraData(this->extraData);
}

TreeNode *TreeNode::get_branch(const std::wstring &_path,bool create){
	if (!_path.size() || _path[0]=='/' || _path[0]=='\\')
		//Path is absolute and to a file in the file system. Do nothing.
		return 0;
	TreeNode *_this=this;
	std::wstring path=_path;
	while (1){
		size_t slash=find_slash(path);
		std::wstring name=path.substr(0,slash);
		if (!name.size())
			return _this;
		bool is_dir=(slash!=path.npos);
		bool stay=1;
		container_iterator i;
		if (name!=L"."){
			stay=0;
			i=_this->branches.find(name);
			if (i==_this->branches.end()){
				if (!create)
					return 0;
				_this->branches[name]=TreeNode(name);
				i=_this->branches.find(name);
				i->second.is_dir=is_dir;
			}
		}
		if (slash!=path.npos){
			while (slash<path.size() && (path[slash]=='/' || path[slash]=='\\'))
				slash++;
			name=path.substr(slash);
		}else
			return &(i->second);
		path=name;
		if (!stay)
			_this=&(i->second);
	}
	return 0;
}

bool Archive::exists(const std::wstring &path){
	TreeNode *node=this->root.get_branch(path,0);
	return !!node;
}

TreeNode *Archive::find_file(const std::wstring &path){
	return (this->good)?this->root.get_branch(path,0):0;
}

bool Archive::read_raw_bytes(void *dst,size_t read_bytes,size_t &bytes_read,const std::wstring &path,Uint64 offset){
	TreeNode *node=this->find_file(path);
	if (!node)
		return 0;
	return this->read_raw_bytes(dst,read_bytes,bytes_read,node,offset);
}

bool Archive::get_file_size(Uint64 &size,const std::wstring &path){
	if (!this->good)
		return 0;
	TreeNode *node=this->root.get_branch(path,0);
	if (!node)
		return 0;
	size=this->get_size(node);
	return 1;
}

template <typename T>
ulong readBigEndian(size_t size,void *_src,T &offset){
	ulong res=0;
	uchar *src=(uchar *)_src;
	src+=offset;
	for (int a=size;a;a--,src++)
		res=(res<<8)|*src;
	offset+=size;
	return res;
}

void writeLittleEndian(size_t size,void *_src,ulong src,size_t &offset){
	for (int a=size;a;a--,offset++){
		((uchar *)_src)[offset]=(uchar)src;
		src>>=8;
	}
}

NSAarchive::NSAarchive(const std::wstring &path,bool nsa)
		:Archive(),
		file(path,1),
		nsa(nsa){
	this->path=path;
	this->root.freeExtraData=NSAarchive::freeExtraData;
	if (!this->file)
		return;
	size_t l;
	char *buffer=(char *)this->file.read(6,l,0);
	if (l<6){
		delete[] buffer;
		return;
	}
	ulong offset=0;
	ulong n=readBigEndian(2,buffer,offset),
		file_data_start=readBigEndian(4,buffer,offset);
	delete[] buffer;
	l=file_data_start-offset;
	buffer=(char *)this->file.read(l,l,offset);
	if (l<file_data_start-offset){
		delete[] buffer;
		return;
	}
	offset=0;
	for (ulong a=0;a<n;a++){
		TreeNode *new_node;
		{
			std::string temp=buffer+offset;
			offset+=temp.size()+1;
			new_node=this->root.get_branch(UniFromSJIS(temp),1);
		}
		NSAdata extraData;
		if (nsa)
			extraData.compression=(NSAdata::compression_type)readByte(buffer,offset);
		else
			extraData.compression=NSAdata::COMPRESSION_NONE;
		extraData.data_offset=file_data_start+readBigEndian(4,buffer,offset);
		extraData.compressed=readBigEndian(4,buffer,offset);
		if (nsa)
			extraData.uncompressed=readBigEndian(4,buffer,offset);
		else
			extraData.uncompressed=extraData.compressed;
		new_node->extraData=new NSAdata(extraData);
		new_node->freeExtraData=NSAarchive::freeExtraData;
	}
	delete[] buffer;
	this->good=1;
}

void NSAarchive::freeExtraData(void *p){
	if (p)
		delete &derefED(p);
}

inline void writeByte(void *_src,ulong src,size_t &offset){
	((uchar *)_src)[offset++]=(uchar)src;
}

uchar *decode_SPB(uchar *buffer,ulong compressedSize,ulong decompressedSize){
	ulong ioffset=0;
	ulong width=readBigEndian(2,buffer,ioffset),
		height=readBigEndian(2,buffer,ioffset);
	ulong width_pad=(4-width*3%4)%4,
		original_length=(width*3+width_pad)*height+54;
	uchar *res=new uchar[original_length];
	memset(res,0,original_length);
	size_t ooffset=0;
	writeByte(res,'B',ooffset);
	writeByte(res,'M',ooffset);
	writeLittleEndian(4,res,original_length,ooffset);
	ooffset=10;
	writeByte(res,54,ooffset);
	ooffset=14;
	writeByte(res,40,ooffset);
	ooffset=18;
	writeLittleEndian(4,res,width,ooffset);
	writeLittleEndian(4,res,height,ooffset);
	writeLittleEndian(2,res,1,ooffset);
	writeByte(res,24,ooffset);
	ooffset=34;
	writeLittleEndian(4,res,original_length-54,ooffset);
	uchar ibitoffset=0;
	uchar *buf=res+54;
	ulong surface=width*height,
		decompressionbufferlen=surface+4;
	uchar *decompressionbuffer=new uchar[decompressionbufferlen];
	ooffset=54;
	for (int a=0;a<3;a++){
		ulong count=0;
		uchar x=(uchar)getbits(buffer,8,&ioffset,&ibitoffset);
		decompressionbuffer[count++]=x;
		while (count<surface){
			uchar n=(uchar)getbits(buffer,3,&ioffset,&ibitoffset);
			if (!n){
				for (int b=4;b;b--)
					decompressionbuffer[count++]=x;
				continue;
			}
			uchar m;
			if (n==7)
				m=getbit(buffer,&ioffset,&ibitoffset)+1;
			else
				m=n+2;
			for (int b=4;b;b--){
				if (m==8)
					x=(uchar)getbits(buffer,8,&ioffset,&ibitoffset);
				else{
					ulong k=(ulong)getbits(buffer,m,&ioffset,&ibitoffset);
					if (k&1)
						x+=uchar((k>>1)+1);
					else
						x-=uchar(k>>1);
				}
				decompressionbuffer[count++]=x;
			}
		}
		uchar *pbuf=buf+(width*3+width_pad)*(height-1)+a;
		uchar *psbuf=decompressionbuffer;
		for (ulong b=0;b<height;b++){
			if (b&1){
				for (ulong c=0;c<width;c++,pbuf-=3)
					*pbuf=*psbuf++;
				pbuf-=width*3+width_pad-3;
			}else{
				for (ulong c=0;c<width;c++,pbuf+=3)
					*pbuf=*psbuf++;
				pbuf-=width*3+width_pad+3;
			}
		}
		long b=0;
		for (long y0=height-1;y0>=0;y0--){
			if (y0&1){
				for (ulong x0=0;x0<width;x0++)
					buf[a+x0*3+y0*(width*3+width_pad)]=decompressionbuffer[b++];
			}else{
				for (long x0=(long)width-1;x0>=0;x0--)
					buf[a+x0*3+y0*(width*3+width_pad)]=decompressionbuffer[b++];
			}
		}
	}
	delete[] decompressionbuffer;
	return res;
}

bool NSAarchive::read_raw_bytes(void *dst,size_t read_bytes,size_t &bytes_read,TreeNode *node,Uint64 offset){
	const NSAdata &nd=derefED(node->extraData);
	if (offset>=nd.compressed)
		read_bytes=0;
	else if (offset+read_bytes>nd.compressed)
		read_bytes=size_t(nd.compressed-offset);
	if (!read_bytes){
		bytes_read=0;
		return 1;
	}
	offset+=nd.data_offset;
	this->file.read(dst,read_bytes,bytes_read,offset);
	return 1;
}

struct endOfCDR{
	bool good;
	ulong disk_number,
		CD_start_disk;
	Uint64 CD_entries_n_disk,	//(total number of entries in the central directory on this disk)
		CD_entries_n,			//(total number of entries in the central directory)
		CD_size,
		CD_start;
	bool use_zip64;
	ulong ECD_start_disk;
	Uint64 ECD_start;
	endOfCDR():good(0){}
	endOfCDR(NONS_File &file,Uint64 offset);
	bool init(NONS_File &file,Uint64 offset);
	bool init64(NONS_File &file,Uint64 offset);
private:
	bool _init64(NONS_File &file,Uint64 offset);
};

endOfCDR::endOfCDR(NONS_File &file,Uint64 offset){
	this->init(file,offset);
}

bool endOfCDR::init(NONS_File &file,Uint64 offset){
	this->good=0;
	size_t l;
	std::vector<uchar> buffer(22);
	uchar *buffer_p=&buffer[0];
	if (!file.read(buffer_p,22,l,offset) || l<22)
		return 0;
	if (ZIParchive::getSignatureType(buffer_p)!=ZIParchive::EOCDR)
		return 0;
	ulong offset2=4;
	this->disk_number=			 readWord(buffer_p,offset2);
	this->CD_start_disk=		 readWord(buffer_p,offset2);
	this->CD_entries_n_disk=	 readWord(buffer_p,offset2);
	this->CD_entries_n=			 readWord(buffer_p,offset2);
	this->CD_size=				readDWord(buffer_p,offset2);
	this->CD_start=				readDWord(buffer_p,offset2);
	size_t comment_l=			 readWord(buffer_p,offset2);

	assert(offset2==22);
	buffer.resize(comment_l);
	if (buffer.size() && (!file.read(&buffer[0],comment_l,l,offset+offset2) || l<comment_l))
		return 0;
	if (
			this->disk_number==0xFFFF ||
			this->CD_start_disk==0xFFFF ||
			this->CD_entries_n_disk==0xFFFF ||
			this->CD_entries_n==0xFFFF ||
			this->CD_size==0xFFFFFFFF ||
			this->CD_start==0xFFFFFFFF)
		return this->_init64(file,offset-20);
	this->use_zip64=0;
	this->good=1;
	return 1;
}

bool endOfCDR::init64(NONS_File &file,Uint64 offset){
	this->good=0;
	size_t l;
	std::vector<uchar> buffer(56);
	uchar *buffer_p=&buffer[0];
	if (!file.read(buffer_p,56,l,offset) || l<56)
		return 0;
	if (ZIParchive::getSignatureType(buffer_p)!=ZIParchive::EOCDR64)
		return 0;
	ulong offset2=4;
	Uint64 ecd64_size=			readQWord(buffer_p,offset2);
	if (ecd64_size<44)
		return 0;
								 readWord(buffer_p,offset2);
								 readWord(buffer_p,offset2);
	this->disk_number=			readDWord(buffer_p,offset2);
	this->CD_start_disk=		readDWord(buffer_p,offset2);
	this->CD_entries_n_disk=	readQWord(buffer_p,offset2);
	this->CD_entries_n=			readQWord(buffer_p,offset2);
	this->CD_size=				readQWord(buffer_p,offset2);
	this->CD_start=				readQWord(buffer_p,offset2);
	this->good=1;
	return 1;
}

bool endOfCDR::_init64(NONS_File &file,Uint64 offset){
	this->good=0;
	this->use_zip64=1;
	size_t l;
	std::vector<uchar> buffer(20);
	uchar *buffer_p=&buffer[0];
	if (!file.read(buffer_p,20,l,offset) || l<20)
		return 0;
	if (ZIParchive::getSignatureType(buffer_p)!=ZIParchive::EOCDR64_LOCATOR)
		return 0;
	ulong offset2=4;
	this->ECD_start_disk=		readDWord(buffer_p,offset2);
	this->ECD_start=			readQWord(buffer_p,offset2);
	this->disk_number=			readDWord(buffer_p,offset2)-1;
	this->good=1;
	return 1;
}

struct centralHeader{
	std::string filename;
	bool good;
	ulong bit_flag,
		compression_method;
	Uint32 crc32;
	Uint64 compressed_size,
		uncompressed_size,
		local_header_off;
	ulong disk_number_start;
	static const Uint16 ZIP64_header_ID=0x0001;
	centralHeader(NONS_File &file,Uint64 &offset,bool &enough_room);
};

#define PUSH_INTO_TEMP(x)\
	if (header_length<(x))\
		continue;\
	temp.push_back(readQWord(buffer_p,offset2))
#define POP_FROM_TEMP(x,y)\
	if ((x)==0xFFFFFFFF && temp.size()){\
		(x)=(y)temp.front();\
		temp.pop_front();\
	}

centralHeader::centralHeader(NONS_File &file,Uint64 &offset,bool &enough_room){
	this->good=0;
	size_t l;
	enough_room=0;
	Uint64 offset_temp=offset;
	std::vector<uchar> buffer(46);
	uchar *buffer_p=&buffer[0];
	if (!file.read(buffer_p,46,l,offset_temp) || l<46)
		return;
	if (ZIParchive::getSignatureType(&buffer[0])==ZIParchive::CENTRAL_HEADER){
		//read main header
		ulong offset2=4;
									 readWord(buffer_p,offset2);
									 readWord(buffer_p,offset2);
		this->bit_flag=				 readWord(buffer_p,offset2);
		this->compression_method=	 readWord(buffer_p,offset2);
									 readWord(buffer_p,offset2);
									 readWord(buffer_p,offset2);
		this->crc32=				readDWord(buffer_p,offset2);
		this->compressed_size=		readDWord(buffer_p,offset2);
		this->uncompressed_size=	readDWord(buffer_p,offset2);
		size_t filename_l=			 readWord(buffer_p,offset2);
		size_t extra_field_l=		 readWord(buffer_p,offset2);
		size_t file_comment_l=		 readWord(buffer_p,offset2);
		this->disk_number_start=	 readWord(buffer_p,offset2);
									 readWord(buffer_p,offset2);
									readDWord(buffer_p,offset2);
		this->local_header_off=		readDWord(buffer_p,offset2);

		assert(offset2==46);

		//read filename
		buffer.resize(filename_l);
		offset_temp+=offset2;
		if (buffer.size()){
			buffer_p=&buffer[0];
			if (!file.read(&buffer[0],filename_l,l,offset_temp) || l<filename_l)
				return;
			this->filename.assign((char *)buffer_p,filename_l);
		}else
			this->filename.clear();
		//read extra field
		buffer.resize(extra_field_l);
		offset_temp+=filename_l;
		if (buffer.size()){
			buffer_p=&buffer[0];
			if (!file.read(buffer_p,extra_field_l,l,offset_temp) || l<extra_field_l)
				return;
			//look for ZIP64 header in extra field
			ulong offset2_temp;
			ulong header_length;
			std::list<Uint64> temp;
			for (offset2=0;offset2<extra_field_l;offset2=offset2_temp+header_length){
				ulong header_type=readWord(buffer_p,offset2);
				header_length=readWord(buffer_p,offset2);
				offset2_temp=offset2;
				if (header_type!=centralHeader::ZIP64_header_ID)
					continue;
				PUSH_INTO_TEMP(8);
				PUSH_INTO_TEMP(16);
				PUSH_INTO_TEMP(24);
				PUSH_INTO_TEMP(28);
				break;
			}
			POP_FROM_TEMP(this->uncompressed_size,Uint64);
			POP_FROM_TEMP(this->compressed_size,Uint64);
			POP_FROM_TEMP(this->local_header_off,Uint64);
			POP_FROM_TEMP(this->disk_number_start,ulong);
			assert(temp.size()==0);
		}
		buffer.resize(file_comment_l);
		offset_temp+=extra_field_l;
		if (buffer.size() && (!file.read(&buffer[0],file_comment_l,l,offset_temp) || l<file_comment_l))
			return;
		offset=offset_temp;
		enough_room=1;
		this->good=1;
	}
}

struct localHeader{
	std::string filename;
	bool good;
	ulong bit_flag,
		compression_method;
	Uint32 crc32;
	Uint64 compressed_size,
		uncompressed_size,
		data_offset;
	localHeader():good(0){}
	localHeader(NONS_File &file,Uint64 offset);
};

localHeader::localHeader(NONS_File &file,Uint64 offset){
	this->good=0;
	size_t l;
	Uint64 offset_temp=offset;
	std::vector<uchar> buffer(30);
	uchar *buffer_p=&buffer[0];
	if (!file.read(buffer_p,30,l,offset_temp) || l<30)
		return;
	if (ZIParchive::getSignatureType(buffer_p)==ZIParchive::LOCAL_HEADER){
		ulong offset2=4;
									 readWord(buffer_p,offset2);
		this->bit_flag=				 readWord(buffer_p,offset2);
		this->compression_method=	 readWord(buffer_p,offset2);
									 readWord(buffer_p,offset2);
									 readWord(buffer_p,offset2);
		this->crc32=				readDWord(buffer_p,offset2);
		this->compressed_size=		readDWord(buffer_p,offset2);
		this->uncompressed_size=	readDWord(buffer_p,offset2);
		size_t filename_l=			 readWord(buffer_p,offset2),
			extra_field_l=			 readWord(buffer_p,offset2);

		assert(offset2==30);

		//read filename
		offset_temp+=offset2;
		buffer.resize(filename_l);
		if (buffer.size()){
			buffer_p=&buffer[0];
			if (!file.read(buffer_p,filename_l,l,offset_temp) || l<filename_l)
				return;
			this->filename.assign((char *)buffer_p,filename_l);
		}else
			this->filename.clear();
		buffer.resize(extra_field_l);
		offset_temp+=filename_l;
		if (buffer.size()){
			buffer_p=&buffer[0];
			if (!file.read(buffer_p,extra_field_l,l,offset_temp) || l<extra_field_l)
				return;
			//look for ZIP64 header in extra field
			ulong offset2_temp;
			ulong header_length;
			std::list<Uint64> temp;
			for (offset2=0;offset2<extra_field_l;offset2=offset2_temp+header_length){
				ulong header_type=readWord(buffer_p,offset2);
				header_length=readWord(buffer_p,offset2);
				offset2_temp=offset2;
				if (header_type!=centralHeader::ZIP64_header_ID)
					continue;
				PUSH_INTO_TEMP(8);
				PUSH_INTO_TEMP(16);
				PUSH_INTO_TEMP(24);
				PUSH_INTO_TEMP(28);
				break;
			}
			POP_FROM_TEMP(this->uncompressed_size,Uint64);
			POP_FROM_TEMP(this->compressed_size,Uint64);
		}
		offset_temp+=extra_field_l;
		offset=this->data_offset=offset_temp;
		this->good=1;
	}
}

#define ZIP_FLAG_UTF8 0x800

ZIParchive::ZIParchive(const std::wstring &path)
		:Archive(){
	this->path=path;
	this->root.freeExtraData=ZIParchive::freeExtraData;
	endOfCDR eocdr;
	NONS_File file;
	file.open(path,1);
	Uint64 filesize;
	Uint64 offset;
	if (!file || (filesize=file.filesize())<22)
		return;
	offset=filesize-22;
	eocdr.init(file,offset);
	for (ulong a=0;a<0x10000 && !eocdr.good && offset;a++)
		eocdr.init(file,--offset);
	if (!eocdr.good)
		return;
	std::wstring name=path;
	size_t dot=name.rfind('.');
	if (dot!=name.npos){
		this->disks.clear();
		name=name.substr(0,dot+1);
		name.push_back('z');
		for (ulong a=0;a<eocdr.disk_number;a++)
			this->disks.push_back(name+itoaw(a+1,2));
	}
	this->disks.push_back(path);
	file.close();
	if (eocdr.use_zip64){
		file.open(this->disks[eocdr.ECD_start_disk],1);
		eocdr.init64(file,eocdr.ECD_start);
		file.close();
		if (!eocdr.good)
			return;
	}
	file.open(this->disks[eocdr.CD_start_disk],1);
	if (!file)
		return;
	filesize=file.filesize();
	offset=eocdr.CD_start;
	if (filesize<=offset)
		return;
	bool fail=0;
	for (ulong entry=0,fileno=eocdr.CD_start_disk;entry<eocdr.CD_entries_n && fileno<this->disks.size() && !fail;entry++){
		bool enough;
		centralHeader temp(file,offset,enough);
		if (!temp.good){
			if (!enough && fileno+1<eocdr.disk_number){
				file.close();
				file.open(this->disks[++fileno],1);
				entry--;
			}else
				fail=1;
		}else{
			if (temp.filename[temp.filename.size()-1]!='/'){
				TreeNode *new_node;
				localHeader *lh;
				if (temp.disk_number_start!=fileno){
					NONS_File file2(this->disks[temp.disk_number_start],1);
					lh=new localHeader(file2,temp.local_header_off);
				}else
					lh=new localHeader(file,temp.local_header_off);
				if (lh->good){
					std::wstring filename;
					if (CHECK_FLAG(temp.bit_flag,ZIP_FLAG_UTF8))
						filename=UniFromUTF8(temp.filename);
					else
						filename=UniFromISO88591(temp.filename);
					new_node=this->root.get_branch(filename,1);
					ZIPdata extraData;
					extraData.bit_flag=lh->bit_flag;
					extraData.compression=(ZIPdata::compression_type)lh->compression_method;
					extraData.crc32=lh->crc32;
					extraData.compressed=lh->compressed_size;
					extraData.uncompressed=lh->uncompressed_size;
					extraData.disk=temp.disk_number_start;
					extraData.data_offset=lh->data_offset;
					new_node->extraData=new ZIPdata(extraData);
					new_node->freeExtraData=ZIParchive::freeExtraData;
				}
				delete lh;
			}
		}
	}
	this->good=!fail;
}

decompression_f select_function(ZIPdata::compression_type c){
	switch (c){
		case ZIPdata::COMPRESSION_DEFLATE:
			return DecompressDEFLATE;
		case ZIPdata::COMPRESSION_BZ2:
			return DecompressBZ2;
		case ZIPdata::COMPRESSION_LZMA:
			return DecompressLZMA;
		default:
			return 0;
	}
}

decompression_f select_function(NSAdata::compression_type c){
	switch (c){
		case NSAdata::COMPRESSION_DEFLATE:
			return DecompressDEFLATE;
		case NSAdata::COMPRESSION_BZ2:
			return DecompressBZ2;
		default:
			return 0;
	}
}

bool ZIParchive::read_raw_bytes(void *dst,size_t read_bytes,size_t &bytes_read,TreeNode *node,Uint64 offset){
	const ZIPdata &zd=derefED(node->extraData);
	Uint64 internal_size=zd.compressed;
	if (offset>=internal_size)
		read_bytes=0;
	else if (offset+read_bytes>internal_size)
		read_bytes=size_t(zd.compressed-offset);
	if (!read_bytes){
		bytes_read=0;
		return 1;
	}
	//find where the file stream section begins
	Uint64 internal_offset=zd.data_offset;
	ulong first_disk=zd.disk;
	while (1){
		Uint64 disk_size;
		NONS_File::get_file_size(disk_size,this->disks[first_disk]);
		if (internal_offset+offset>=disk_size){
			first_disk++;
			offset-=disk_size-internal_offset;
			internal_offset=0;
			continue;
		}
		break;
	}
	//read until the destination buffer is full
	bytes_read=0;
	while (read_bytes){
		NONS_File file(this->disks[first_disk++],1);
		size_t temp;
		file.read(dst,read_bytes,temp,internal_offset+offset);
		bytes_read+=temp;
		read_bytes-=temp;
		dst=((uchar *)dst)+temp;
		internal_offset=0;
		offset=0;
	}
	return 1;
}

void ZIParchive::freeExtraData(void *p){
	if (p)
		delete &derefED(p);
}

ZIParchive::SignatureType ZIParchive::getSignatureType(void *buffer){
	ulong offset=0;
	Uint32 s=readDWord((char *)buffer,offset);
	switch (s){
		case ZIParchive::local_signature:
			return ZIParchive::LOCAL_HEADER;
		case ZIParchive::central_signature:
			return ZIParchive::CENTRAL_HEADER;
		case ZIParchive::EOCDR_signature:
			return ZIParchive::EOCDR;
		case ZIParchive::EOCDR64_locator_signature:
			return ZIParchive::EOCDR64_LOCATOR;
		case ZIParchive::EOCDR64_signature:
			return ZIParchive::EOCDR64;
	}
	return ZIParchive::NOT_A_SIGNATURE;
}

static const wchar_t *formats[]={
	L".zip",
	L".oaf",
	L""
};

static const wchar_t name_ext[]=L"abcdefghijklmnopqrstuvwxyz";
static const wchar_t *name_base[]={
	L"patch",
	L"data",
	0
};

void NONS_GeneralArchive::init(){
	static const wchar_t *path=L"./";
	this->archives.push_back(&filesystem);
	for (ulong c=0;name_base[c];c++){
	 	for (ulong a=0;a<=26;a++){
	 	 	std::wstring full_name;
		 	ulong format=0;
		 	std::wstring name=name_base[c];
		 	if(!c)
		 	 	a=26-a;
	 	 	if (a){
		 	 	name.append(L"_");
			 	name.push_back(name_ext[a-1]);
		 	}	
		 	for (ulong b=0;b<3;b++){
			 	full_name=path;
			 	full_name.append(name);
			 	full_name.append(formats[b]);
			 	if (NONS_File::file_exists(full_name) || NONS_File::file_exists(toupperCopy(full_name)))
		 	 	 	this->addArchive_private(full_name,b);
		 	}
		 	if(!c)
		 	 	a=26-a;
		}
	}
	std::reverse(this->archives.begin(),this->archives.end());
}

bool NONS_GeneralArchive::addArchive_private(const std::wstring &path,int format){
	NONS_MutexLocker ml(this->mutex);
	bool reverse=0;
	if (format<0){
		reverse=1;
		for (int a=0;format<0 && a<3;a++)
			if (ends_with(path,(std::wstring)formats[a]))
				format=a;
		if (format<0)
			return 0;
		std::reverse(this->archives.begin(),this->archives.end());
	}
	NONS_ArchiveSource *ds=0;
	switch (format){
		case 0:
		case 1:
			ds=new NONS_zipArchiveSource(path);
			break;
		case 2:
 	  	ds=new NONS_nsaArchiveSource(path,1);
		 	break;
	}
	bool r=ds->good();
	if (r)
		this->archives.push_back(ds);
	else{
		delete ds;
		if (!reverse)
			o_stderr <<"Archive "<<path<<" is invalid.\n";
	}
	if (reverse)
		std::reverse(this->archives.begin(),this->archives.end());
	return r;
}

NONS_GeneralArchive::~NONS_GeneralArchive(){
	if (!this->archives.size())
		return;
	this->archives.pop_back();
	while (this->archives.size()){
		delete this->archives.back();
		this->archives.pop_back();
	}
}

uchar *NONS_GeneralArchive::getFileBuffer(const std::wstring &filepath,size_t &buffersize,bool use_filesystem){
	NONS_MutexLocker ml(this->mutex);
	uchar *res=0;
	size_t end=this->archives.size();
	if (!use_filesystem)
		end--;
	for (size_t a=0;a<end && !res;a++)
		res=this->archives[a]->read_all(filepath,buffersize);
	return res;
}

NONS_DataStream *NONS_GeneralArchive::open(const std::wstring &path,ulong flags){
	bool keep_in_memory=CHECK_FLAG(flags,KEEP_IN_MEMORY),
		filesystem_first=CHECK_FLAG(flags,FILESYSTEM_FIRST);
	NONS_MutexLocker ml(this->mutex);
	size_t n=this->archives.size();
	if (filesystem_first){
		NONS_DataStream *stream=this->archives.back()->open(path,keep_in_memory);
		if (stream)
			return stream;
		n--;
	}
	for (size_t a=0;a<n;a++){
		NONS_DataStream *stream=this->archives[a]->open(path,keep_in_memory);
		if (stream)
			return stream;
	}
	if (CLOptions.verbosity>=VERBOSITY_LOG_OPEN_FAILURES)
		o_stderr <<"NONS_GeneralArchive::open(): Failed to open stream to \""<<path<<"\".\n";
	return 0;
}

bool NONS_GeneralArchive::close(NONS_DataStream *stream){
	NONS_MutexLocker ml(this->mutex);
	if (stream){
		for (size_t a=0;a<this->archives.size();a++)
			if (this->archives[a]->close(stream))
				return 1;
		if (memoryFS.close(stream))
			return 1;
	}
	return 0;
}

bool NONS_GeneralArchive::exists(const std::wstring &path){
	NONS_MutexLocker ml(this->mutex);
	for (size_t a=0;a<this->archives.size();a++)
		if (this->archives[a]->exists(path))
			return 1;
	return 0;
}

//------------------------------------------------------------------------------
//NONS_ArchiveSource
//------------------------------------------------------------------------------

bool NONS_ArchiveSource::get_size(Uint64 &size,const std::wstring &name){
	return this->archive->get_file_size(size,name);
}

Uint64 NONS_ArchiveSource::get_size(TreeNode *node){
	return this->archive->get_size(node);
}

bool NONS_ArchiveSource::read(void *dst,size_t &bytes_read,NONS_DataStream &stream,size_t count){
	if (!this->archive->read_raw_bytes(dst,count,bytes_read,stream.get_name(),stream.get_offset()))
		return 0;
	stream.seek(bytes_read,0);
	return 1;
}

bool NONS_ArchiveSource::read(void *dst,size_t &bytes_read,NONS_ArchiveStream &stream,size_t count){
	if (!this->archive->read_raw_bytes(dst,count,bytes_read,stream.get_node(),stream.get_offset()))
		return 0;
	stream.seek(bytes_read,0);
	return 1;
}

TreeNode *NONS_ArchiveSource::get_node(const std::wstring &path){
	return this->archive->find_file(path);
}

uchar *NONS_ArchiveSource::read_all(const std::wstring &name,size_t &bytes_read){
	TreeNode *node=this->get_node(name);
	if (!node)
		return 0;
	return this->read_all(node,bytes_read);
}

bool NONS_ArchiveSource::exists(const std::wstring &name){
	return !!this->get_node(name);
}

//------------------------------------------------------------------------------
//NONS_nsaArchiveSource
//------------------------------------------------------------------------------

NONS_DataStream *NONS_nsaArchiveSource::open(const std::wstring &name,bool keep_in_memory){
	NONS_DataStream *stream;
	NONS_nsaArchiveStream *p=new NONS_nsaArchiveStream(*this,name);
	TreeNode *node=p->get_node();
	if (!node){
		delete p;
		return 0;
	}
	NSAdata data=NSAarchive::derefED(node->extraData);
	if (data.compression!=NSAdata::COMPRESSION_NONE){
		delete p;
		stream=0;
		std::wstring path=filesystem.new_temp_name();
		Uint64 offset=0;
		switch (data.compression){
			case NSAdata::COMPRESSION_BZ2:
				offset=4;
			case NSAdata::COMPRESSION_DEFLATE:
				{
					Uint32 a=0;
					if (!keep_in_memory)
						decompress_file_to_file(path,this->archive,node,data,name,a,offset);
					else{
						std::vector<uchar> temp(data.uncompressed);
						decompress_file_to_memory(&temp[0],this->archive,node,data,name,offset);
						stream=memoryFS.new_temporary_file(&temp[0],temp.size());
					}
				}
				break;
			default:
				{
					size_t l;
					uchar *decompressed=this->read_all(node,l);
					if (!keep_in_memory)
						NONS_File::write(path,decompressed,l);
					else
						stream=memoryFS.new_temporary_file(decompressed,data.uncompressed);
					delete[] decompressed;
				}
				break;
		}
		if (!stream)
			stream=filesystem.new_temporary_file(path);
		assert(!!stream);
	}else
		stream=p;
	return NONS_DataSource::open(stream,normalize_path(this->archive->path+L"/"+name));
}

NONS_nsaArchiveSource::NONS_nsaArchiveSource(const std::wstring &name,bool nsa){
	this->archive=new NSAarchive(name,nsa);
}

NONS_nsaArchiveSource::~NONS_nsaArchiveSource(){
	delete this->archive;
}

uchar *NONS_nsaArchiveSource::read_all(TreeNode *node,size_t &bytes_read){
	NSAdata data=NSAarchive::derefED(node->extraData);
	uchar *ret=0;
	bytes_read=data.uncompressed;
	if (data.compression==NSAdata::COMPRESSION_NONE){
		ret=new uchar[(size_t)data.uncompressed];
		size_t l=(size_t)data.compressed;
		this->archive->read_raw_bytes(ret,l,l,node,0);
	}else{
		std::wstring path=filesystem.new_temp_name();
		if (data.compression==NSAdata::COMPRESSION_BZ2){
			ret=new uchar[(size_t)data.uncompressed];
			if (!decompress_file_to_memory(ret,this->archive,node,data,node->name)){
				delete[] ret;
				return 0;
			}
		}else{
			uchar *compressed=new uchar[data.compressed];
			size_t l=data.compressed;
			this->archive->read_raw_bytes(compressed,l,l,node,0);
			switch (data.compression){
				case NSAdata::COMPRESSION_LZSS:
					ret=decode_LZSS(compressed,data.compressed,data.uncompressed);
					break;
				case NSAdata::COMPRESSION_SPB:
					ret=decode_SPB(compressed,data.compressed,data.uncompressed);
					break;
				//Shut GCC up:
				default:;
			}
			delete[] compressed;
		}
	}
	return ret;
}

//------------------------------------------------------------------------------
//NONS_zipArchiveSource
//------------------------------------------------------------------------------

NONS_zipArchiveSource::NONS_zipArchiveSource(const std::wstring &name){
	this->archive=new ZIParchive(name);
}

NONS_zipArchiveSource::~NONS_zipArchiveSource(){
	delete this->archive;
}

Uint32 crc32_from_stream(NONS_DataStream *stream){
	stream->reset();
	CRC32 crc32;
	std::vector<uchar> buffer(1<<12);
	while (1){
		size_t l=buffer.size();
		if (!stream->read(&buffer[0],l,l) || !l)
			break;
		crc32.Input(&buffer[0],l);
	}
	stream->reset();
	return crc32.Result();
}

NONS_DataStream *NONS_zipArchiveSource::open(const std::wstring &name,bool keep_in_memory){
	NONS_DataStream *stream;
	NONS_zipArchiveStream *p=new NONS_zipArchiveStream(*this,name);
	stream=p;
	TreeNode *node=p->get_node();
	if (!node){
		delete p;
		return 0;
	}
	ZIPdata data=ZIParchive::derefED(node->extraData);
	bool skip_crc=data.compressed>50*1024*1024;
	Uint32 crc32=1;
	if (data.compression!=ZIPdata::COMPRESSION_NONE){
		delete p;
		if (!keep_in_memory){
			std::wstring path=filesystem.new_temp_name();
			decompress_file_to_file(path,this->archive,node,data,name,crc32);
			stream=filesystem.new_temporary_file(path);
		}else{
			std::vector<uchar> temp((size_t)data.uncompressed);
			decompress_file_to_memory(&temp[0],this->archive,node,data,name);
			stream=memoryFS.new_temporary_file(&temp[0],temp.size());
			CRC32 crc_temp;
			crc_temp.Input(&temp[0],temp.size());
			crc32=crc_temp.Result();
		}
		assert(!!stream);
	}else if (!skip_crc)
		crc32=crc32_from_stream(stream);
	if (!skip_crc && crc32!=data.crc32){
		o_stderr <<"File "<<name<<" did not pass CRC32 test.\n";
		delete stream;
		return 0;
	}
	return NONS_DataSource::open(stream,normalize_path(this->archive->path+L"/"+name));
}

uchar *NONS_zipArchiveSource::read_all(TreeNode *node,size_t &bytes_read){
	ZIPdata data=ZIParchive::derefED(node->extraData);
	uchar *ret=new uchar[(size_t)data.uncompressed];
	if (data.compression==ZIPdata::COMPRESSION_NONE){
		size_t l=(size_t)data.compressed;
		this->archive->read_raw_bytes(ret,l,l,node,0);
	}else{
		if (!decompress_file_to_memory(ret,this->archive,node,data,node->name)){
			delete[] ret;
			return 0;
		}
	}
	CRC32 crc;
	crc.Input(ret,(size_t)data.uncompressed);
	if (crc.Result()!=data.crc32){
		delete[] ret;
		ret=0;
	}
	return ret;
}

//------------------------------------------------------------------------------
//NONS_ArchiveStream
//------------------------------------------------------------------------------

NONS_ArchiveStream::NONS_ArchiveStream(NONS_DataSource &ds,const std::wstring &name)
		:NONS_DataStream(ds,name){
	this->node=((NONS_ArchiveSource *)this->source)->get_node(this->name);
	if (this->node)
		this->size=((NONS_ArchiveSource *)this->source)->get_size(this->node);
}

//------------------------------------------------------------------------------
//NONS_nsaArchiveStream
//------------------------------------------------------------------------------

bool NONS_nsaArchiveStream::read(void *dst,size_t &bytes_read,size_t count){
	return ((NONS_ArchiveSource *)this->source)->read(dst,bytes_read,*this,count);
}

//------------------------------------------------------------------------------
//NONS_zipArchiveStream
//------------------------------------------------------------------------------

bool NONS_zipArchiveStream::read(void *dst,size_t &bytes_read,size_t count){
	return ((NONS_ArchiveSource *)this->source)->read(dst,bytes_read,*this,count);
}
