/* -*- C++ -*-
 * 
 *  ONScripter_extra.cpp - Execution block parser of ONScripter
 *  This file is added by Taigacon and Gjy
 *
 *  Copyright (c) 2001-2013 Ogapee. All rights reserved.
 *
 *  gjy_1992@qq.com
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

#include "ONScripter.h"

void ONScripter::errorAndExit(const char *str, const char *reason )
{
	char *error;
	time_t t = time(NULL);
    struct tm *tm = localtime( &t );
    char error_name[64];
    sprintf(error_name,"%serror%d-%02d-%02d-%02d%02d%02d.bmp",archive_path,tm->tm_year,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
    SDL_BlitSurface(screen_surface, NULL, screenshot_surface, NULL);

    SDL_Surface *surface =  
AnimationInfo::alloc32bitSurface( script_h.screen_width * screen_ratio1 / screen_ratio2 , script_h.screen_height * screen_ratio1 / screen_ratio2, texture_format );
        resizeSurface( screenshot_surface, surface );
        SDL_SaveBMP( surface, error_name );
        SDL_FreeSurface( surface );
	if ( reason )
        fprintf( stderr, " *** Parse error at %s:%d [%s]; %s ***\n",
                 current_label_info.name,
                 current_line,
                 str, reason );
    else
        fprintf( stderr, " *** Parse error at %s:%d [%s] ***\n",
                 current_label_info.name,
                 current_line,
                 str );
	
    exit(-1);

}