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

#define	DEFAULT_STDOUT_WIDTH	80
#define	DEFAULT_STDOUT_HEIGHT	25

//Colors
#define	BG_BLACK			0x00
#define	FG_BLACK			0x00
#define	BG_WHITE			0x70
#define	FG_BRIGHT_WHITE		0x0F

//Clear screen
void		cls();

//Print string
void		print_string(char* str, unsigned char color,unsigned char bg_color);

//Set cursor position
void		set_cursor_pos(unsigned short line, unsigned short row);

//Write video buffer
void		write_video_buf(
	unsigned short* p_data,		//Data to write
	unsigned long size,			//How mant bytes to write
	//start position
	unsigned short line,
	unsigned short row);


#endif	//! STDOUT_H_INCLUDE
