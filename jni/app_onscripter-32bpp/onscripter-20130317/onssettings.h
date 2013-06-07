/* -*- C++ -*-
 * 
 *  onssetings.h -- Settings File Module for ONScripter
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
 
#ifndef __ONSSETTINGS_H__
#define __ONSSETTINGS_H__

#include "ONScripter.h"


int ReadSettings(ONScripter *pOns, const char *file);

#if defined(PSP) || defined(__PS3__)
extern SDLKey os_button_map[];
#endif

#if defined(PSP)
void PSPInit();
void PSPFin();
#endif

#if defined(PDA) && defined(PDA_WIDTH)
extern int os_screen_width;
#endif



#endif
