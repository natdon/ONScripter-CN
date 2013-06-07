/*
 * Copyright (c) 2004  Kazunori "jagarl" Ueno
 * Copyright (c) 2000, 2001 Yuki Sawada
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <string>

using namespace std;

#include "font.h"
#include "font_peer.h"
#include "codeconv.h"

namespace XKFont {

typedef struct _FontLibrary {
  FT_Library super;
  int ref_count;
  char **paths;
  int num_paths;
} FontLibrary;

static FontLibrary *library = NULL;

static char *default_paths[] = {
  ".",
  "/",
  "/usr/X11R6/lib/X11/fonts/TrueType",
  "/usr/local/share/fonts/TrueType",
  "/usr//share/fonts/TrueType",
  "/usr//share/fonts/tt",
  NULL
};

static void font_library_ft2_add_path(const char *path);
static void font_library_ft2_remove_path(const char *path);
static void font_library_ft2_add_path(const char *path);
static char *font_library_ft2_build_path(const char *base, const char *name);

static int
font_library_ft2_alloc()
{
  int i;
  
  if (!library) {
    library = (FontLibrary*) calloc(sizeof(FontLibrary), 1);
    if (library) {
      if (FT_Init_FreeType(&library->super))
	goto _1;
      fprintf(stderr, "XKFont::font_library_ft2_alloc : FreeType allocated successfully.\n");
      for (i = 0; default_paths[i]; i++)
	font_library_ft2_add_path(default_paths[i]);
    }
  }

  library->ref_count++;
  return 1;

 _1:
  free(library);
  library = NULL;
  fprintf(stderr, "XKFont::font_library_ft2_alloc: Couldn't allocate FreeType.\n");
  return 0;
}

static void
font_library_ft2_free()
{
  int i;
  
  if (!library || library->ref_count <= 0)
    return;
  if (--library->ref_count == 0) {
    FT_Done_FreeType(library->super);
    for (i = 0; i < library->num_paths; i++)
      free(library->paths[i]);
    free(library->paths);
    free(library);
    library = NULL;
    fprintf(stderr, "XKFont::font_library_ft2_free : FreeType done.\n");
  }
}

static void
font_library_ft2_add_path(const char *path)
{
  library->num_paths++;
  if (!library->paths)
    library->paths = (char**) malloc(sizeof(char *));
  else
    library->paths = (char**) realloc( (void*)library->paths,
			     library->num_paths * sizeof(char *));
  library->paths[library->num_paths - 1] = strdup(path);
}

static void
font_library_ft2_remove_path(const char *path)
{
  int i, j;

  for (i = 0; i < library->num_paths; i++) {
    if (!strcmp(path, library->paths[i])) {
      library->num_paths--;
      for (j = i; j < library->num_paths; j++)
	library->paths[j] = library->paths[j + 1];
      if (library->num_paths > 0)
	library->paths = (char**) realloc(library->paths,
				 library->num_paths * sizeof(char *));
      else {
	free(library->paths);
	library->paths = NULL;
      }
    }
  }
}

typedef struct _FontEncoding {
  FT_UShort platform_id;
  FT_Encoding encoding;
  FontCodeConverter conv_func;
} FontEncoding;

static FontEncoding encodings[] = {
  { 3, (FT_Encoding)ft_encoding_unicode, codeconv_euc_to_unicode },
  { 3, (FT_Encoding)ft_encoding_sjis, codeconv_euc_to_sjis },
  { 1, (FT_Encoding)ft_encoding_apple_roman, codeconv_euc_to_latin1 },
  { (FT_UShort)-1, (FT_Encoding)-1, NULL }
};

static char *
font_library_ft2_build_path(const char *base, const char *name)
{
  char *path;
  const char *strs[] = { base, "/", name, NULL };
  int i = 0;
  
  path = (char*) calloc(sizeof(char), strlen(base) + strlen(name) + 2);
  if (path) 
    while (strs[i])
      strcat(path, strs[i++]);
    
  return path;
}

PeerFt2::PeerFt2(const char *name, int index, int hsize, int vsize)
{
  int i,j;

  font_library_ft2_alloc();
  
  for (i = 0; i < library->num_paths; i++) {
    char *path = font_library_ft2_build_path(library->paths[i], name);
    if (path) {
	if (FT_New_Face(library->super, path, index, &face))
	  face = NULL;
	free(path);
    }
    if (face)
	break;
  }
  
  if (!face) {
	string err = string("XKFont::PeerFt2::PeerFt : Cannot open font(TrueType) : ")+name;
	throw std::invalid_argument(err);
  }
  
  conv_func = 0;
  for (i=0; encodings[i].conv_func != 0; i++) {
	FT_UShort platform_id = encodings[i].platform_id;
    	FT_Encoding encoding = encodings[i].encoding;
    	for (j = 0; j < face->num_charmaps; j++) {
      		FT_CharMap cmap = face->charmaps[j];
      		if (cmap->platform_id == platform_id && cmap->encoding == encoding) {
      			if (FT_Set_Charmap(face, cmap) == 0) {
       				conv_func = encodings[i].conv_func;
				break;
      			}
    		}
    	}
	if (conv_func) break;
  }
  if (conv_func == 0) {
	FT_Done_Face(face);
	fprintf(stderr,"cannot find charmap\n");
	string err = string("XKFont::PeerFt2::PeerFt : No supported code converter of font (TrueType) ")+name;
	throw std::invalid_argument(err);
  }
  FT_Set_Pixel_Sizes(face, hsize, vsize);

}

PeerFt2::~PeerFt2() {
	FT_Done_Face(face);
	font_library_ft2_free();
}

bool
PeerFt2::GlyphCreate(unsigned int code, Glyph* glyph)
{
  FT_GlyphSlot slot;
  FT_UInt index;
  int bmsize;

  if (face == 0) return false;
  code = conv_func(code);
  if (code == 0) return false;
  index = FT_Get_Char_Index(face, code);
  if (index == 0) return false;

  /* Don't consider error */
  slot = face->glyph;
  if (slot) {
    // if (! FT_Load_Glyph(face, index, FT_LOAD_DEFAULT)) {
// BITMAP だと なぜか render してくれない……
// LOAD_DEFAULT でも、下に対応コードを付けたので一応は大丈夫
    if (! FT_Load_Glyph(face, index, FT_LOAD_NO_BITMAP)) {
    	FT_Render_Glyph(slot, ft_render_mode_normal);
    }
  }
  
  glyph->bitmap_left = slot->bitmap_left;
  glyph->bitmap_top = slot->bitmap_top;
  glyph->bitmap.width = slot->bitmap.width;
  glyph->bitmap.rows = slot->bitmap.rows;

/*
  glyph->metrics.ascender = private->face->size->metrics.ascender >> 6;
  glyph->metrics.descender = private->face->size->metrics.descender >> 6;
*/

  glyph->advance.x = slot->advance.x >> 6;
  glyph->advance.y = slot->advance.y >> 6;

  bmsize = glyph->bitmap.width * glyph->bitmap.rows;
  if (bmsize > 0) {
    glyph->bitmap.buffer = new unsigned char[bmsize];
    memcpy(glyph->bitmap.buffer, slot->bitmap.buffer, bmsize);
  }
// なぜか Render したのに MONO なことがある……
/* for freetype < 2.1.3, use ``ft_pixel_mode_mono'' */
if (slot->bitmap.pixel_mode == ft_pixel_mode_mono) {int i,j;
// if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {int i,j;
	char* d = (char*)slot->bitmap.buffer;
	for (i=0; i<glyph->bitmap.rows; i++) {
		int flag = *d++; int len = 8;
		unsigned char* buf = glyph->bitmap.buffer + i*slot->bitmap.width;
		for (j=0; j<glyph->bitmap.width; j++) {
			if (len == 0) {
				flag = *d++;
				len = 8;
			}
			if (flag & 0x80) *buf++ = 0xff;
			else *buf++ = 0;
			flag <<= 1;
			len--;
		}
	}
}

  return true;

}

} /* end of namespace XKFont */

