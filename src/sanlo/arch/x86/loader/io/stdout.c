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

static	void			scroll_down(unsigned short line,unsigned char color);

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


void print_string(char* str, unsigned char color,unsigned char bg_color)
{
	char* p;
	unsigned short character;
	unsigned short p_video_mem;
	
	for(p=str;*p!='\0';p++){
		if(p == '\n'){
			current_cursor_line++;
			current_cursor_row=0;
			if(current_cursor_line>=DEFAULT_STDOUT_HEIGHT){
				//Scroll down
				current_cursor_line--;
				scroll_down(1,bg_color);
			}
			set_cursor_pos(current_cursor_line,current_cursor_row);
		}else{
			//Print character
			character=(unsigned short)color<<8+*p
			p_video_mem=
				(unsigned short*)BASIC_VIDEO_BASE_ADDR+current_cursor_line*current_cursor_row;
			*p_video_mem=character;
			current_cursor_row++;
			if(current_cursor_row>=DEFAULT_STDOUT_WIDTH){
				current_cursor_row=0;
				current_cursor_line++;
				if(current_cursor_line>=DEFAULT_STDOUT_HEIGHT){
					//Scroll down
					current_cursor_line--;
					scroll_down(1,bg_color);
				}
			}
			set_cursor_pos(current_cursor_line,current_cursor_row);
		}
	}
	return;
}

void set_cursor_pos(unsigned short line, unsigned short row)
{
	unsigned short pos;
	
	//Check the range of position
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
void write_video_buf(
	unsigned short* p_data,		//Data to write
	unsigned long size,			//How many bytes to write
	//start position
	unsigned short line,
	unsigned short row)
{
	unsigned short offset;
	unsigned short* p_dest;
	offset = line * row * 2;
	
	//Check bound
	if(offset + size 
		> DEFAULT_STDOUT_HEIGHT * DEFAULT_STDOUT_WIDTH*2){
		return;
	}
	size = size \ 2 * 2;
	
	//Write buf
	p_dest = (char*)BASIC_VIDEO_BASE_ADDR + offset;
	memcpy(p_dest,p_data,size);
	return;
}

void scroll_down(unsigned short line,unsigned char color)
{
	unsigned short offset;
	unsigned short len;
	
	if(line >= DEFAULT_STDOUT_HEIGHT){
		cls();
		return;
	}
	
	offset=(line-1)*DEFAULT_STDOUT_WIDTH*2;
	len=DEFAULT_STDOUT_HEIGHT * DEFAULT_STDOUT_WIDTH*2-offset;
	
	__asm__ __volatile__(
		"cld\n\t"
		"movl		%0,%%edi\n\t"
		"movl		%0,%%esi\n\t"
		"movl		%1,eax%%\n\t"
		"addl		%%eax,%%esi\n\t"
		"movl		%2,%%ecx\n\t"
		"rep		movsb\n\t"
		"movl		%0,%%edi\n\t"
		"movl		%2,%%eax\n\t"
		"addl		%%eax,%%edi\n\t"
		"movl		%1,%%ecx\n\t"
		"movb		%3,%%ah\n\t"
		"movb		$0x20,%%al\n\t"
		"rep		stosb\n\t"
		::"i"(BASIC_VIDEO_BASE_ADDR),"m"(offset),"m"(len),"m"(color));
	return;
}