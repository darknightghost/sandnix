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

static	unsigned short	current_cursor_line = 0;
static	unsigned short	current_cursor_row = 0;



void cls(u8 color)
{
	size_t size;
	size = DEFAULT_STDOUT_WIDTH * DEFAULT_STDOUT_HEIGHT;
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%2,%%edi\n\t"
	    "movl		%0,%%ecx\n\t"
	    "movb		%1,%%ah\n\t"
	    "movb		$0x20,%%al\n\t"
	    "rep		stosw"
	    ::"m"(size), "m"(color), "i"(BASIC_VIDEO_BASE_ADDR));
	GET_REAL_VARIABLE(current_cursor_line) = 0;
	GET_REAL_VARIABLE(current_cursor_row) = 0;
	set_cursor_pos(0, 0);
	return;
}


void print_string(char* str, u8 color, u8 bg_color)
{
	char* p;
	u16 character;
	u16 offset;
	u16* p_video_mem;

	for(p = str; *p != '\0'; p++) {
		if(*p == '\n') {
			GET_REAL_VARIABLE(current_cursor_line)++;
			GET_REAL_VARIABLE(current_cursor_row) = 0;

			if(GET_REAL_VARIABLE(current_cursor_line) >= DEFAULT_STDOUT_HEIGHT) {
				//Scroll down
				GET_REAL_VARIABLE(current_cursor_line)--;
				scroll_down(1, bg_color);
			}

			set_cursor_pos(
			    GET_REAL_VARIABLE(current_cursor_line),
			    GET_REAL_VARIABLE(current_cursor_row));

		} else if(*p == '\t') {
			print_string(GET_REAL_ADDR("    "), color, bg_color);

		} else {
			//Print character
			character = (u16)color * 0x100 + *p;
			offset =
			    (GET_REAL_VARIABLE(current_cursor_line) * DEFAULT_STDOUT_WIDTH
			     + GET_REAL_VARIABLE(current_cursor_row))
			    * 2;
			p_video_mem = (u16*)((u8*)BASIC_VIDEO_BASE_ADDR + offset);
			*p_video_mem = character;
			GET_REAL_VARIABLE(current_cursor_row)++;

			if(GET_REAL_VARIABLE(current_cursor_row) >= DEFAULT_STDOUT_WIDTH) {
				GET_REAL_VARIABLE(current_cursor_row) = 0;
				GET_REAL_VARIABLE(current_cursor_line)++;

				if(GET_REAL_VARIABLE(current_cursor_line) >= DEFAULT_STDOUT_HEIGHT) {
					//Scroll down
					GET_REAL_VARIABLE(current_cursor_line)--;
					scroll_down(1, bg_color);
				}
			}

			set_cursor_pos(
			    GET_REAL_VARIABLE(current_cursor_line),
			    GET_REAL_VARIABLE(current_cursor_row));
		}
	}

	return;
}

void set_cursor_pos(u16 line, u16 row)
{
	u16 pos;

	//Check the range of position
	if(line >= DEFAULT_STDOUT_HEIGHT
	   || row >= DEFAULT_STDOUT_WIDTH) {
		return;
	}

	pos = line * DEFAULT_STDOUT_WIDTH + row;
	GET_REAL_VARIABLE(current_cursor_line) = line;
	GET_REAL_VARIABLE(current_cursor_row) = row;
	//Disable interruptions
	__asm__ __volatile__(
	    "pushf\n\t"
	    "cli\n\t");
	out_byte((u8)CURSOR_POS_H_REG, (u16)CRTC_ADDR_REG);
	out_byte((u8)((pos >> 8) & 0xFF), (u16)CRTC_DATA_REG);
	out_byte((u8)CURSOR_POS_L_REG, (u16)CRTC_ADDR_REG);
	out_byte((u8)(pos & 0xFF), (u16)CRTC_DATA_REG);
	__asm__ __volatile__(
	    "popf\n\t");
	return;
}

void get_cursor_pos(u16* line, u16* row)
{
	*line = current_cursor_line;
	*row = current_cursor_row;
	return;
}

void write_video_buf(
    u16* p_data,
    size_t size,
    u16 line,
    u16 row)
{
	u16 offset;
	u16* p_dest;
	offset = line * row * 2;

	//Check bound
	if(offset + size
	   > DEFAULT_STDOUT_HEIGHT * DEFAULT_STDOUT_WIDTH * 2) {
		return;
	}

	size = (size_t)(size / 2) * 2;
	//Write buf
	p_dest = (u16*)((u8*)BASIC_VIDEO_BASE_ADDR + offset);
	memcpy(p_dest, p_data, size);
	return;
}

void scroll_down(u16 line, u16 color)
{
	u16 offset;
	u16 len;
	u16	half_len;

	if(line >= DEFAULT_STDOUT_HEIGHT) {
		cls(color);
		return;
	}

	offset = line * DEFAULT_STDOUT_WIDTH * 2;
	len = DEFAULT_STDOUT_HEIGHT * DEFAULT_STDOUT_WIDTH * 2 - offset;
	half_len = offset / 2;
	__asm__ __volatile__(
	    "cld\n\t"
	    "push		%%es\n\t"
	    "push		%%ds\n\t"
	    "movw		%%gs,%%ax\n\t"
	    "movw		%%ax,%%es\n\t"
	    "movw		%%ax,%%ds\n\t"
	    "movzwl		%0,%%esi\n\t"
	    "xorl		%%edi,%%edi\n\t"
	    "movzwl		%1,%%ecx\n\t"
	    "rep		movsb\n\t"
	    "movzwl		%1,%%edi\n\t"
	    "movzwl		%3,%%ecx\n\t"
	    "movb		%2,%%ah\n\t"
	    "movb		$0x20,%%al\n\t"
	    "rep		stosw\n\t"
	    "pop		%%ds\n\t"
	    "pop		%%es\n\t"
	    ::"m"(offset), "m"(len), "m"(color), "m"(half_len));
	return;
}
