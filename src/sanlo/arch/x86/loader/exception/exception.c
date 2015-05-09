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
#include "../io/keyboard.h"

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
	SET_EXCEPTION_STR(EXCEPTION_UNKNOW_EXCEPTION	, "EXCEPTION_UNKNOW_EXCEPTION");
	SET_EXCEPTION_STR(EXCEPTION_HEAP_CORRUPTION		, "EXCEPTION_HEAP_CORRUPTION");
	SET_EXCEPTION_STR(EXCEPTION_DE					, "EXCEPTION_DE");
	SET_EXCEPTION_STR(EXCEPTION_DB					, "EXCEPTION_DB");
	SET_EXCEPTION_STR(EXCEPTION_NMI					, "EXCEPTION_NMI");
	SET_EXCEPTION_STR(EXCEPTION_BR					, "EXCEPTION_BR");
	SET_EXCEPTION_STR(EXCEPTION_UD					, "EXCEPTION_UD");
	SET_EXCEPTION_STR(EXCEPTION_NM					, "EXCEPTION_NM");
	SET_EXCEPTION_STR(EXCEPTION_DF					, "EXCEPTION_DF");
	SET_EXCEPTION_STR(EXCEPTION_FPU					, "EXCEPTION_FPU");
	SET_EXCEPTION_STR(EXCEPTION_TS					, "EXCEPTION_TS");
	SET_EXCEPTION_STR(EXCEPTION_NP					, "EXCEPTION_NP");
	SET_EXCEPTION_STR(EXCEPTION_SS					, "EXCEPTION_SS");
	SET_EXCEPTION_STR(EXCEPTION_GP					, "EXCEPTION_GP");
	SET_EXCEPTION_STR(EXCEPTION_PF					, "EXCEPTION_PF");
	SET_EXCEPTION_STR(EXCEPTION_RESERVED			, "EXCEPTION_RESERVED");
	SET_EXCEPTION_STR(EXCEPTION_MF					, "EXCEPTION_MF");
	SET_EXCEPTION_STR(EXCEPTION_AC					, "EXCEPTION_AC");
	SET_EXCEPTION_STR(EXCEPTION_MC					, "EXCEPTION_MC");
	SET_EXCEPTION_STR(EXCEPTION_XF					, "EXCEPTION_XF");
	SET_EXCEPTION_STR(EXCEPTION_NOT_ENOUGH_MEMORY	, "EXCEPTION_NOT_ENOUGH_MEMORY");
	SET_EXCEPTION_STR(EXCEPTION_NO_CONFIG_FILE		, "EXCEPTION_NO_CONFIG_FILE");
	SET_EXCEPTION_STR(EXCEPTION_UNEXPECT_CONFIG_FILE, "EXCEPTION_UNEXPECT_CONFIG_FILE");
	SET_EXCEPTION_STR(EXCEPTION_NO_KERNEL			, "EXCEPTION_NO_KERNEL");
	SET_EXCEPTION_STR(EXCEPTION_UNKNOW_KERNEL_FORMAT, "EXCEPTION_UNKNOW_KERNEL_FORMAT");
	SET_EXCEPTION_STR(EXCEPTION_KERNEL_PARAMETER_TOO_LONG, "EXCEPTION_KERNEL_PARAMETER_TOO_LONG");
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
	        "The Sandnix loader cannot continue running,Press <ESC> to restart your computer."),
	    BG_BLACK | FG_BRIGHT_RED,
	    BG_BLACK);

	while(get_keyboard_input() != KEY_ESC_PRESSED);

	__asm__ __volatile__(
	    "movb	$0xFE,%al\n\t"
	    "outb	%al,$0x64\n\t");
	return;
}
#pragma GCC pop_options
