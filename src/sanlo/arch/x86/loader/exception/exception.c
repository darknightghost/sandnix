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
	case (num):\
		exception_str=GET_REAL_ADDR(value);\
		break;\
	}

void panic(u32 reason)
{
	char* exception_str;

	switch(reason) {
		SET_EXCEPTION_STR(0, "EXCEPTION_UNKNOW_EXCEPTION");
		SET_EXCEPTION_STR(1, "EXCEPTION_HEAP_CORRUPTION");

	default:
		exception_str = GET_REAL_ADDR("UNKNOW");
	}

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
