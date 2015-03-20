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

char*	exception_table[] = {
	"UNKNOW EXCEPTION OCCURED",
	"HEAP CORRUPTION DETECTED"
};

void panic(u32 reason)
{
	cls(BG_BLACK | FG_BRIGHT_RED);
	print_string(
		"Function panic() has been called.\n",
		BG_BLACK | FG_BRIGHT_RED,
		BG_BLACK);
	print_string(
		exception_table[reason],
		BG_BLACK | FG_BRIGHT_RED,
		BG_BLACK);
	print_string(
		"\nAn unhandled exception has been occured,please restart your computer.\n",
		BG_BLACK | FG_BRIGHT_RED,
		BG_BLACK);

	while(1);

	return;
}
