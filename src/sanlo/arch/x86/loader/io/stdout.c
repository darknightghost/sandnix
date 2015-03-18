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

#include "stdout.h"
#include "io.h"
#include "../segment.h"
#include "../string/string.h"

#define	CRTC_ADDR_REG		0x03D4
#define	CRTC_DATA_REG		0x03D5
#define	CURSOR_POS_H_REG	0x0E
#define	CURSOR_POS_L_REG	0x0F
#define	START_ADDR_H_REG	0x0C
#define	START_ADDR_L_REG	0x0D

static unsigned short current_cursor_line = 0;
static unsigned short current_cursor_row = 0;

void cls(unsigned char color)
{
	unsigned long size;
	size = DEFAULT_STDOUT_WIDTH * DEFAULT_STDOUT_HEIGHT;
	__asm__ __volatile__(
		"cld\n\t"
		"movl		%2,%%edi\n\t"
		"movl		%0,%%ecx\n\t"
		"movb		%1,%%ah\n\t"
		"movb		$0x20,%%al\n\t"
		"rep		stosw"
		::"m"(size), "m"(color), "i"(BASIC_VIDEO_BASE_ADDR));
	current_cursor_line = 0;
	current_cursor_row = 0;
	set_cursor_pos(0, 0);
	return;
}


void print_string(char* str, unsigned char color)
{
	return;
}

void set_cursor_pos(unsigned short line, unsigned short row)
{
	unsigned short pos;

	if(line >= DEFAULT_STDOUT_HEIGHT
	   || row >= DEFAULT_STDOUT_WIDTH) {
		return;
	}

	pos = line * DEFAULT_STDOUT_WIDTH + row;
	current_cursor_line = line;
	current_cursor_row = row;
	//Disable interruptions
	__asm__ __volatile__(
		"pushfd\n\t"
		"cli\n\t");
	out_byte(CRTC_ADDR_REG, CURSOR_POS_H_REG);
	out_byte(CRTC_DATA_REG, (pos >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_POS_L_REG);
	out_byte(CRTC_DATA_REG, pos & 0xFF);
	__asm__ __volatile__(
		"popfd\n\t");
	return;
}

//Write video buf
void		write_video_buf(
	unsigned short* p_data,		//Data to write
	unsigned long size,			//How mant bytes to write
	//start position
	unsigned short line,
	unsigned short row);

