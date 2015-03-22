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

#include "exception.h"
#include "../io/stdout.h"

#define	SET_EXCEPTION_STR(num,value) {\
		if(reason==(num)){\
			exception_str=GET_REAL_ADDR(value);\
			goto print_err;\
		}\
	}

#pragma GCC push_options
#pragma GCC optimize ("O0")
void panic(u32 reason)
{
	char* exception_str;
	__asm__ __volatile__(
		"cli\n\t"
	);
	SET_EXCEPTION_STR(0x00000000, "EXCEPTION_UNKNOW_EXCEPTION");
	SET_EXCEPTION_STR(0x00000001, "EXCEPTION_HEAP_CORRUPTION");
	SET_EXCEPTION_STR(0x00000002, "EXCEPTION_DE");
	SET_EXCEPTION_STR(0x00000003, "EXCEPTION_DB");
	SET_EXCEPTION_STR(0x00000004, "EXCEPTION_NMI");
	SET_EXCEPTION_STR(0x00000005, "EXCEPTION_BR");
	SET_EXCEPTION_STR(0x00000006, "EXCEPTION_UD");
	SET_EXCEPTION_STR(0x00000007, "EXCEPTION_NM");
	SET_EXCEPTION_STR(0x00000008, "EXCEPTION_DF");
	SET_EXCEPTION_STR(0x00000009, "EXCEPTION_FPU");
	SET_EXCEPTION_STR(0x0000000A, "EXCEPTION_TS");
	SET_EXCEPTION_STR(0x0000000B, "EXCEPTION_NP");
	SET_EXCEPTION_STR(0x0000000C, "EXCEPTION_SS");
	SET_EXCEPTION_STR(0x0000000D, "EXCEPTION_GP");
	SET_EXCEPTION_STR(0x0000000E, "EXCEPTION_PF");
	SET_EXCEPTION_STR(0x0000000F, "EXCEPTION_RESERVED");
	SET_EXCEPTION_STR(0x00000010, "EXCEPTION_MF");
	SET_EXCEPTION_STR(0x00000011, "EXCEPTION_AC");
	SET_EXCEPTION_STR(0x00000012, "EXCEPTION_MC");
	SET_EXCEPTION_STR(0x00000013, "EXCEPTION_XF");
	//Default
	exception_str = GET_REAL_ADDR("UNKNOW");
print_err:
	cls(BG_BLACK | FG_BRIGHT_RED);
	print_string(
		GET_REAL_ADDR("Function panic() has been called.\n\n"),
		BG_BLACK | FG_BRIGHT_RED,
		BG_BLACK);
	print_string(
		exception_str,
		BG_BLACK | FG_BRIGHT_RED,
		BG_BLACK);
	print_string(
		GET_REAL_ADDR(
			"\n\nThis function is called when an unhandled exception has been occured.\n"),
		BG_BLACK | FG_BRIGHT_RED,
		BG_BLACK);
	print_string(
		GET_REAL_ADDR(
			"The Sandnix loader cannot continue running,please restart your computer."),
		BG_BLACK | FG_BRIGHT_RED,
		BG_BLACK);

	while(1);

	return;
}
#pragma GCC pop_options
