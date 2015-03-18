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

#include "io.h"

//Read port
unsigned char in_byte(unsigned short port)
{
	unsigned char data;
	__asm__ __volatile__(
		"movw		%1,%%dx\n\t"
		"inb		%%dx,%%al\n\t"
		:"=%%al"(data)
		:"m"(port));
	return data;
}

unsigned short in_word(unsigned short port)
{
	unsigned short data;
	__asm__ __volatile__(
		"movw		%1,%%dx\n\t"
		"inw		%%dx,%%ax\n\t"
		:"=%%ax"(data)
		:"m"(port));
	return data;
}

//Write port
void out_byte(unsigned char data, unsigned short port)
{
	__asm__ __volatile__(
		"movw		%1,%%dx\n\t"
		"movb		%0,%%al\n\t"
		"outb		%%al,%%dx\n\t"
		::"m"(data), "m"(port));
	return;
}

void out_word(unsigned short data, unsigned short port)
{
	__asm__ __volatile__(
		"movw		%1,%%dx\n\t"
		"movw		%0,%%ax\n\t"
		"outw		%%ax,%%dx\n\t"
		::"m"(data), "m"(port));
	return;
}