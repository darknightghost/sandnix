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

#include "../io/io.h"
#include "../debug/debug.h"
#include "../pm/pm.h"
#include "stdio.h"

#define	SCAN_CODE(code,c)	case	(code):\
	if(shift_count>0){\
		dbg_print("%c",(c)-0x20);\
		rtl_queue_push(&char_queue,(void*)((c)-0x20),NULL);\
	}else{\
		dbg_print("%c",(c));\
		rtl_queue_push(&char_queue,(void*)(c),NULL);\
	}\
	break;

static	queue_t				char_queue;
static	int_hndlr_info_t	info;
static	u32					shift_count;

static	bool				keyboard_int_func(u32 int_num, u32 thread_id, u32 err_code);
static	void				analyse_scan_code(u8 scan_code);

void stdio_init()
{
	io_set_crrnt_int_level(INT_LEVEL_USR_HIGHEST);
	rtl_queue_init(&char_queue);
	info.func = keyboard_int_func;
	io_reg_int_hndlr(IRQ1, &info);
	shift_count = 0;
	io_read_port_byte(0x60);
	io_read_port_byte(0x60);
	return;
}


bool keyboard_int_func(u32 int_num, u32 thread_id, u32 err_code)
{
	u8 scan_code;

	scan_code = io_read_port_byte(0x60);
	analyse_scan_code(scan_code);

	io_read_port_byte(0x60);
	io_write_port_byte(0x20, 0x20);

	UNREFERRED_PARAMETER(int_num);
	UNREFERRED_PARAMETER(thread_id);
	UNREFERRED_PARAMETER(err_code);
	return true;
}

void analyse_scan_code(u8 scan_code)
{

	switch(scan_code) {
	case 0x2A:
	case 0x36:
		//Shift down
		shift_count++;
		break;

	case 0xAA:
	case 0xB6:
		//Shift up
		shift_count--;
		break;
		SCAN_CODE(0x1E, 'a');
		SCAN_CODE(0x30, 'b');
		SCAN_CODE(0x2E, 'c');
		SCAN_CODE(0x20, 'd');
		SCAN_CODE(0x12, 'e');
		SCAN_CODE(0x21, 'f');
		SCAN_CODE(0x22, 'g');
		SCAN_CODE(0x23, 'h');
		SCAN_CODE(0x17, 'i');
		SCAN_CODE(0x24, 'j');
		SCAN_CODE(0x25, 'k');
		SCAN_CODE(0x26, 'l');
		SCAN_CODE(0x32, 'm');
		SCAN_CODE(0x31, 'n');
		SCAN_CODE(0x18, 'o');
		SCAN_CODE(0x19, 'p');
		SCAN_CODE(0x10, 'q');
		SCAN_CODE(0x13, 'r');
		SCAN_CODE(0x1F, 's');
		SCAN_CODE(0x14, 't');
		SCAN_CODE(0x16, 'u');
		SCAN_CODE(0x2F, 'v');
		SCAN_CODE(0x11, 'w');
		SCAN_CODE(0x2D, 'x');
		SCAN_CODE(0x15, 'y');
		SCAN_CODE(0x2C, 'z');

	case 0x35:
		dbg_print("%c", '/');
		rtl_queue_push(&char_queue, (void*)'/', NULL);
		break;

	case 0x1C:
		dbg_print("%c", '\n');
		rtl_queue_push(&char_queue, (void*)'\n', NULL);
		break;

	case 0x39:
		dbg_print("%c", ' ');
		rtl_queue_push(&char_queue, (void*)' ', NULL);
		break;

	case 0x0B:
		dbg_print("%c", '0');
		rtl_queue_push(&char_queue, (void*)'0', NULL);
		break;

	case 0x02:
		dbg_print("%c", '1');
		rtl_queue_push(&char_queue, (void*)'1', NULL);
		break;

	case 0x03:
		dbg_print("%c", '2');
		rtl_queue_push(&char_queue, (void*)'2', NULL);
		break;

	case 0x04:
		dbg_print("%c", '3');
		rtl_queue_push(&char_queue, (void*)'3', NULL);
		break;

	case 0x05:
		dbg_print("%c", '4');
		rtl_queue_push(&char_queue, (void*)'4', NULL);
		break;

	case 0x06:
		dbg_print("%c", '5');
		rtl_queue_push(&char_queue, (void*)'5', NULL);
		break;

	case 0x07:
		dbg_print("%c", '6');
		rtl_queue_push(&char_queue, (void*)'6', NULL);
		break;

	case 0x08:
		dbg_print("%c", '7');
		rtl_queue_push(&char_queue, (void*)'7', NULL);
		break;

	case 0x09:
		dbg_print("%c", '8');
		rtl_queue_push(&char_queue, (void*)'8', NULL);
		break;

	case 0x0A:
		dbg_print("%c", '9');
		rtl_queue_push(&char_queue, (void*)'9', NULL);
		break;
	}

	return;
}

void read_line(char* buf)
{
	char* p;

	p = buf;

	while(1) {

		if(rtl_queue_front(&char_queue) != NULL) {
			*p = (char)(u32)(rtl_queue_pop(&char_queue, NULL));

			if(*p == '\n') {
				*p = '\0';
				return;
			}

			p++;
		}

	}
}
