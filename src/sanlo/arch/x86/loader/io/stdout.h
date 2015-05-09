/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

	This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef	STDOUT_H_INCLUDE
#define	STDOUT_H_INCLUDE

#include "../types.h"

#define	DEFAULT_STDOUT_WIDTH	80
#define	DEFAULT_STDOUT_HEIGHT	25

//Colors
#define	BG_BLACK			0x00
#define	BG_RED				0x40
#define	BG_WHITE			0x70
#define	FG_BLACK			0x00
#define	FG_BRIGHT_RED		0x0C
#define	FG_BRIGHT_WHITE		0x0F

void		cls(u8 color);
void		print_string(char* str, u8 color, u8 bg_color);
void		set_cursor_pos(u16 line, u16 row);
void		get_cursor_pos(u16* line, u16* row);
void		write_video_buf(
    u16* p_data,				//Data to write
    size_t size,				//How mant bytes to write
    //start position
    u16 line,
    u16 row);
void		scroll_down(u16 line, u16 color);


#endif	//! STDOUT_H_INCLUDE
