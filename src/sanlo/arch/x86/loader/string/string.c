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

#include "string.h"

void memset(char* addr, size_t size, char value)
{
	__asm__ __volatile__(
		"cld\n\t"
		"movl		%0,%%edi\n\t"
		"movl		%1,%%ecx\n\t"
		"movb		%2,%%al\n\t"
		"rep		stosb\n\t"
		::"m"(addr), "m"(size), "m"(value));
	return;
}
