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

#include "../../debug.h"
#include "../../../mm/mm.h"
#include "../../../rtl/rtl.h"
#include "../../../io/io.h"
#include "../../../setup/setup.h"
#include "../../../pm/pm.h"
#include "../../../setup/setup.h"

#define	DEFAULT_STDOUT_WIDTH	80
#define	DEFAULT_STDOUT_HEIGHT	25

#define	BG_BLACK			0x00
#define	BG_RED				0x40
#define	BG_WHITE			0x70
#define	FG_BLACK			0x00
#define	FG_BRIGHT_RED		0x0C
#define	FG_BRIGHT_WHITE		0x0F

#define	CRTC_ADDR_REG		0x03D4
#define	CRTC_DATA_REG		0x03D5
#define	CURSOR_POS_H_REG	0x0E
#define	CURSOR_POS_L_REG	0x0F
#define	START_ADDR_H_REG	0x0C
#define	START_ADDR_L_REG	0x0D

#define	BUF_SIZE	4096

static	char	k_dbg_tty_buf[K_TTY_BUF_SIZE];
static	char*	p_tty_buf = NULL;
static	bool	enable_print_flag = true;

static	unsigned short	current_cursor_line = 0;
static	unsigned short	current_cursor_row = 0;

static	void		print_string(char* str, u8 color, u8 bg_color);
static	void		set_cursor_pos(u16 line, u16 row);
static	void		scroll_down(u16 line, u16 color);
static	spinlock_t	print_lock;

void dbg_init()
{
	pm_init_spn_lock(&print_lock);
	return;
}

void dbg_print(char* fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	dbg_vprint(fmt, args);
	va_end(args);
}

void dbg_vprint(char* fmt, va_list args)
{
	char* buf;
	size_t len;

	pm_acqr_spn_lock(&print_lock);
	buf = mm_hp_alloc(BUF_SIZE, NULL);
	rtl_vprintf_s(buf, BUF_SIZE, fmt, args);

	//Wrtie string to buf;
	if(p_tty_buf == NULL) {
		p_tty_buf = k_dbg_tty_buf;
	}

	len = rtl_strlen(buf);

	if(K_TTY_BUF_SIZE - (p_tty_buf - k_dbg_tty_buf) < len) {
		rtl_memmove(
		    k_dbg_tty_buf,
		    k_dbg_tty_buf + len - (K_TTY_BUF_SIZE - (p_tty_buf - k_dbg_tty_buf)),
		    len - (K_TTY_BUF_SIZE - (p_tty_buf - k_dbg_tty_buf)));
		p_tty_buf = p_tty_buf - (len - (K_TTY_BUF_SIZE - (p_tty_buf - k_dbg_tty_buf)));
	}

	rtl_memcpy(p_tty_buf, buf, len);
	p_tty_buf += len;

	if(p_tty_buf >= k_dbg_tty_buf + K_TTY_BUF_SIZE) {
		excpt_panic(ENOMEM,
		            "There's not enought memory for k_dbg_tty_buf!\n");
	}

	//Print string
	if(enable_print_flag) {
		print_string(buf, FG_BRIGHT_WHITE, BG_BLACK);
	}

	mm_hp_free(buf, NULL);
	pm_rls_spn_lock(&print_lock);
	return;
}

void dbg_cls()
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
	    ::"m"(size), "i"(FG_BRIGHT_WHITE | BG_BLACK), "i"(BASIC_VIDEO_BASE_ADDR));
	current_cursor_line = 0;
	current_cursor_row = 0;
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
			current_cursor_line++;
			current_cursor_row = 0;

			if(current_cursor_line >= DEFAULT_STDOUT_HEIGHT) {
				//Scroll down
				current_cursor_line--;
				scroll_down(1, bg_color);
			}

			set_cursor_pos(
			    current_cursor_line,
			    current_cursor_row);

		} else if(*p == '\t') {
			if(current_cursor_row % 4 == 0) {
				print_string(" ", color, bg_color);
			}

			while(current_cursor_row % 4 != 0) {
				print_string(" ", color, bg_color);
			}

		} else {
			//Print character
			character = (u16)(color | bg_color) * 0x100 + *p;
			offset =
			    (current_cursor_line * DEFAULT_STDOUT_WIDTH
			     + current_cursor_row)
			    * 2;
			p_video_mem = (u16*)((u8*)BASIC_VIDEO_BASE_ADDR + offset);
			*p_video_mem = character;
			current_cursor_row++;

			if(current_cursor_row >= DEFAULT_STDOUT_WIDTH) {
				current_cursor_row = 0;
				current_cursor_line++;

				if(current_cursor_line >= DEFAULT_STDOUT_HEIGHT) {
					//Scroll down
					current_cursor_line--;
					scroll_down(1, bg_color);
				}
			}

			set_cursor_pos(
			    current_cursor_line,
			    current_cursor_row);
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
	current_cursor_line = line;
	current_cursor_row = row;

	//Set cursor position
	io_write_port_byte((u8)CURSOR_POS_H_REG, (u16)CRTC_ADDR_REG);
	io_write_port_byte((u8)((pos >> 8) & 0xFF), (u16)CRTC_DATA_REG);
	io_write_port_byte((u8)CURSOR_POS_L_REG, (u16)CRTC_ADDR_REG);
	io_write_port_byte((u8)(pos & 0xFF), (u16)CRTC_DATA_REG);
	return;
}

void scroll_down(u16 line, u16 color)
{
	u16 offset;
	u16 len;
	u16 half_len;

	if(line >= DEFAULT_STDOUT_HEIGHT) {
		dbg_cls(color);
		return;
	}

	offset = line * DEFAULT_STDOUT_WIDTH * 2;
	len = DEFAULT_STDOUT_HEIGHT * DEFAULT_STDOUT_WIDTH * 2 - offset;
	half_len = offset / 2;
	__asm__ __volatile__(
	    "movzwl		%0,%%esi\n\t"
	    "addl		%4,%%esi\n\t"
	    "movl		%4,%%edi\n\t"
	    "movzwl		%1,%%ecx\n\t"
	    "rep		movsb\n\t"
	    "movzwl		%1,%%edi\n\t"
	    "addl		%4,%%edi\n\t"
	    "movzwl		%3,%%ecx\n\t"
	    "movb		%2,%%ah\n\t"
	    "movb		$0x20,%%al\n\t"
	    "rep		stosw\n\t"
	    ::"m"(offset), "m"(len), "m"(color),
	    "m"(half_len), "i"(BASIC_VIDEO_BASE_ADDR));
	return;
}
