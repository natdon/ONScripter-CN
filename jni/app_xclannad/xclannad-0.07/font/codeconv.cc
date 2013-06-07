/*
 * Copyright (c) 2000 Yuki Sawada
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

#include "codeconv.h"
#include "cp936.h"
#include "codeconv_tbl.h"

static unsigned int
codeconv_euc_to_jis(unsigned int euc)
{
  unsigned int hi, low;

  hi = (euc >> 8) & 0xff;
  low = euc & 0xff;

  if (hi < 0x81) {
    hi = 0;
  } else if (low == 0x8e)
    hi = 0;
  else {
    hi -= 0x80;
    low -= 0x80;
  }

  return (hi << 8) | low;
}

static unsigned int
codeconv_jis_to_unicode(unsigned int jis)
{
  int k0, k1;

  if (jis < 0x80) return jis; // ASCII
  k0 = (jis >> 8) - 0x20;
  if (k0 < 1 || k0 > 92)
    return 0;

  k1 = (jis % 0x100) - 0x20;
  if (k1 < 1 || k1 > 94)
    return 0;

  return unicode_tbl[k0 - 1][k1 - 1];
}

unsigned int
codeconv_euc_to_unicode(unsigned int euc)
{
  unsigned int jis, unicode,unicode2;

 // jis = codeconv_euc_to_jis(euc);
  //unicode = codeconv_jis_to_unicode(jis);
    //unicode = zz_gbk2uni((euc>>8)&0xFF,euc&0xFF);
    Cp936 cp;
    unicode = cp.JisDecode(euc);
    unicode2 = cp.Convert(unicode);

	if(unicode2 == 0x8cb2)
	{
		unicode2 = 0x670b;
	}
	else if (unicode2 == 0x803d)
	{
		unicode2 = 0x0028;
	}
	else if (unicode2 == 0x864e)
	{
		unicode2 = 0x300c;
	}
	else if (unicode2 == 0x7d77)
	{
		unicode2 = 0x5d0e;
	}
    
  return unicode2;
}

static unsigned int
codeconv_unicode_to_jis(unsigned int unicode)
{
  int k0, k1;
  unsigned int jis;

  k0 = (unicode >> 8) & 0xff;
  k1 = unicode & 0xff;
  jis = unicode_rev_table[k0][k1];
    
  return jis;
}

static unsigned int
codeconv_jis_to_euc(unsigned int jis)
{
  unsigned int hi, low;

  hi = (jis >> 8) & 0x7f | 0x80;
  low = jis & 0x7f | 0x80;

  return (hi << 8) | low;
}

unsigned int
codeconv_unicode_to_euc(unsigned int unicode)
{
    return unicode;
  unsigned int jis, euc;

  if (unicode >= 0xff61 && unicode <= 0xff9f)
    return unicode - 0xff61 + 0x8ea1;

  jis = codeconv_unicode_to_jis(unicode);
  if (jis == 0)
    return 0x7878;
  euc = codeconv_jis_to_euc(jis);

  return euc;
}

static unsigned int
codeconv_jis_to_sjis(unsigned int jis)
{
  unsigned int hi, low;

  hi = (jis >> 8) & 0xff;
  low = jis & 0xff;

  low += (hi & 0x01) ? 0x1f : 0x7d;
  if (low >= 0x7f)
    low++;
  hi = ((hi - 0x21) >> 1) + 0x81;
  if (hi > 0x9f)
    hi += 0x40;

  return (hi << 8) | low;
}

unsigned int
codeconv_euc_to_sjis(unsigned int euc)
{
    return euc;
  unsigned int jis, sjis;

  jis = codeconv_euc_to_jis(euc);
  sjis = codeconv_jis_to_sjis(jis);

  return sjis;
}

static unsigned int
codeconv_sjis_to_jis(unsigned int sjis)
{
  unsigned int hi, low;

  hi = (sjis >> 8) & 0xff;
  low = sjis & 0xff;

  hi -= (hi <= 0x9f) ? 0x71 : 0xb1;
  hi = (hi << 1) + 1;
  if (low > 0x7f)
    low--;
  if (low >= 0x9e) {
    low -= 0x7d;
    hi++;
  } else
    low -= 0x1f;
  
  return (hi << 8) | low;
}

unsigned int
codeconv_sjis_to_euc(unsigned int sjis)
{
    return sjis;
  unsigned int jis, euc;

  jis = codeconv_sjis_to_jis(sjis);
  euc = codeconv_jis_to_euc(jis);

  return euc;
}

unsigned int
codeconv_euc_to_latin1(unsigned int euc)
{
	int high = (euc>>8) & 0xff;
	if (high) return 0;
	return euc & 0xff;
}
