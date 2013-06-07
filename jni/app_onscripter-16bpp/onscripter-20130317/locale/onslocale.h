/* -*- C++ -*-
 *
 *  onslocale.h - Locale Layer for ONScripter
 *
 *  Copyright (c) 2007 John_He. All rights reserved.
 *
 *  john_he_@163.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __ONSLOCALE_H__
#define __ONSLOCALE_H__

/* Message Index */
/*enum
{
	LOCSTR_DEFAULT_SAVE_MENU_NAME = 0,
	LOCSTR_DEFAULT_LOAD_MENU_NAME,
	LOCSTR_DEFAULT_SAVE_ITEM_NAME,
	LOCSTR_MESSAGE_SAVE_EXIST_1B,
	LOCSTR_MESSAGE_SAVE_EMPTY_1B,
	LOCSTR_MESSAGE_SAVE_CONFIRM_1B,
	LOCSTR_MESSAGE_LOAD_CONFIRM_1B,
	LOCSTR_MESSAGE_RESET_CONFIRM_1B,
	LOCSTR_MESSAGE_END_CONFIRM_1B,
	LOCSTR_MESSAGE_YES_1B,
	LOCSTR_MESSAGE_NO_1B,
	LOCSTR_MESSAGE_SAVE_EXIST,
	LOCSTR_MESSAGE_SAVE_EMPTY,
	LOCSTR_MESSAGE_SAVE_CONFIRM,
	LOCSTR_MESSAGE_LOAD_CONFIRM,
	LOCSTR_MESSAGE_RESET_CONFIRM,
	LOCSTR_MESSAGE_END_CONFIRM,
	LOCSTR_MESSAGE_YES,
	LOCSTR_MESSAGE_NO
};*/
	

/* Redirect macros to functions */
#define IS_TWO_BYTE(x) (onsLocaleIsTwoByte((unsigned char)(x)))
#define IS_KINSOKU(x) (onsLocaleIsKinsoku((const unsigned char *)(x)))
#define IS_ROTATION_REQUIRED(x) (onsLocaleIsRotationRequired((const unsigned char *)(x)))
#define IS_TRANSLATION_REQUIRED(x) (onsLocaleIsTranslationRequired((const unsigned char *)(x)))

#define DEFAULT_SAVE_MENU_NAME (onsLocaleGetString(LOCSTR_DEFAULT_SAVE_MENU_NAME))
#define DEFAULT_LOAD_MENU_NAME (onsLocaleGetString(LOCSTR_DEFAULT_LOAD_MENU_NAME))
#define DEFAULT_SAVE_ITEM_NAME (onsLocaleGetString(LOCSTR_DEFAULT_SAVE_ITEM_NAME))
/*#if defined(ENABLE_1BYTE_CHAR) && defined(FORCE_1BYTE_CHAR)
#define MESSAGE_SAVE_EXIST (onsLocaleGetString(LOCSTR_MESSAGE_SAVE_EXIST_1B))
#define MESSAGE_SAVE_EMPTY (onsLocaleGetString(LOCSTR_MESSAGE_SAVE_EMPTY_1B))
#define MESSAGE_SAVE_CONFIRM (onsLocaleGetString(LOCSTR_MESSAGE_SAVE_CONFIRM_1B))
#define MESSAGE_LOAD_CONFIRM (onsLocaleGetString(LOCSTR_MESSAGE_LOAD_CONFIRM_1B))
#define MESSAGE_RESET_CONFIRM (onsLocaleGetString(LOCSTR_MESSAGE_RESET_CONFIRM_1B))
#define MESSAGE_END_CONFIRM (onsLocaleGetString(LOCSTR_MESSAGE_END_CONFIRM_1B))
#define MESSAGE_YES (onsLocaleGetString(LOCSTR_MESSAGE_YES_1B))
#define MESSAGE_NO (onsLocaleGetString(LOCSTR_MESSAGE_NO_1B))
#else
#define MESSAGE_SAVE_EXIST (onsLocaleGetString(LOCSTR_MESSAGE_SAVE_EXIST))
#define MESSAGE_SAVE_EMPTY (onsLocaleGetString(LOCSTR_MESSAGE_SAVE_EMPTY))
#define MESSAGE_SAVE_CONFIRM (onsLocaleGetString(LOCSTR_MESSAGE_SAVE_CONFIRM))
#define MESSAGE_LOAD_CONFIRM (onsLocaleGetString(LOCSTR_MESSAGE_LOAD_CONFIRM))
#define MESSAGE_RESET_CONFIRM (onsLocaleGetString(LOCSTR_MESSAGE_RESET_CONFIRM))
#define MESSAGE_END_CONFIRM (onsLocaleGetString(LOCSTR_MESSAGE_END_CONFIRM))
#define MESSAGE_YES (onsLocaleGetString(LOCSTR_MESSAGE_YES))
#define MESSAGE_NO (onsLocaleGetString(LOCSTR_MESSAGE_NO))
#endif*/

#define LOC_TWOBYTE_SYMBOL(x) (onsLocaleGet2BAsciiSymbolStr() + (((x)-0x20)*2))
#define LOC_TWOBYTE_NUM_STR (LOC_TWOBYTE_SYMBOL('0'))


const char *onsLocaleGetString(int index);
const char *onsLocaleGet2BAsciiSymbolStr();
int onsLocaleIsTwoByte(unsigned char x); 
int onsLocaleIsKinsoku(const unsigned char *x);
int onsLocaleIsRotationRequired(const unsigned char *x);
int onsLocaleIsTranslationRequired(const unsigned char *x);
void onsLocaleInit(void);
unsigned short onsLocaleConv(unsigned short in);

#endif
