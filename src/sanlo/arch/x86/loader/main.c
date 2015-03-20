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

#include "io/io.h"
#include "io/stdout.h"
#include "exception/exception.h"
#include "segment.h"

void	c_main()
{
	u16 character=0x0C*0x100+'A';
	u16 offset=2;
	cls(BG_BLACK | FG_BRIGHT_WHITE);
	/*print_string(
		"Protect mode entered.\nSearching for sanlo.cfg...\n",
		FG_BRIGHT_WHITE | BG_BLACK,
		BG_BLACK);*/
	__asm__ __volatile__(
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"movzwl		%1,%%ebx\n\t"
		"movw		%0,%%ax\n\t"
		"movw		%%ax,%%gs:0x280(%%ebx)"
		::"m"(character),"m"(offset));
	scroll_down(3,0x20);
	while(1);

	return;
}
