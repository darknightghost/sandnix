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

#include "../../io.h"

void io_read_port_bytes(u32 port, u8* buf, u32 num)
{
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%0,%%edx\n\t"
	    "movl		%1,%%edi\n\t"
	    "movl		%2,%%ecx\n\t"
	    "rep		insb\n\t"
	    ::"m"(port), "m"(buf), "m"(num));
	return;
}

void io_write_port_bytes(u32 port, u8* buf, u32 num)
{
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%0,%%edx\n\t"
	    "movl		%1,%%esi\n\t"
	    "movl		%2,%%ecx\n\t"
	    "rep		outsb\n\t"
	    ::"m"(port), "m"(buf), "m"(num));
	return;
}

void io_read_port_words(u32 port, u16* buf, u32 num)
{
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%0,%%edx\n\t"
	    "movl		%1,%%edi\n\t"
	    "movl		%2,%%ecx\n\t"
	    "rep		insw\n\t"
	    ::"m"(port), "m"(buf), "m"(num));
	return;
}
void io_write_port_words(u32 port, u16* buf, u32 num)
{
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%0,%%edx\n\t"
	    "movl		%1,%%esi\n\t"
	    "movl		%2,%%ecx\n\t"
	    "rep		outsw\n\t"
	    ::"m"(port), "m"(buf), "m"(num));
	return;
}

void io_read_port_dwords(u32 port, u32* buf, u32 num)
{
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%0,%%edx\n\t"
	    "movl		%1,%%edi\n\t"
	    "movl		%2,%%ecx\n\t"
	    "rep		insl\n\t"
	    ::"m"(port), "m"(buf), "m"(num));
	return;
}

void io_write_port_dwords(u32 port, u32* buf, u32 num)
{
	__asm__ __volatile__(
	    "cld\n\t"
	    "movl		%0,%%edx\n\t"
	    "movl		%1,%%esi\n\t"
	    "movl		%2,%%ecx\n\t"
	    "rep		outsl\n\t"
	    ::"m"(port), "m"(buf), "m"(num));
	return;
}

u8 io_read_port_byte(u32 port)
{
	u8 data;
	__asm__ __volatile__(
	    "movw		%1,%%dx\n\t"
	    "inb		%%dx,%%al\n\t"
	    :"=%%al"(data)
	    :"m"(port));
	return data;
}

void io_write_port_byte(u8 data, u32 port)
{
	__asm__ __volatile__(
	    "movw		%1,%%dx\n\t"
	    "movb		%0,%%al\n\t"
	    "outb		%%al,%%dx\n\t"
	    ::"m"(data), "m"(port));
	return;
}

u16 io_read_port_word(u32 port)
{
	u16 data;
	__asm__ __volatile__(
	    "movw		%1,%%dx\n\t"
	    "inw		%%dx,%%ax\n\t"
	    :"=%%ax"(data)
	    :"m"(port));
	return data;
}

void io_write_port_word(u16 data, u32 port)
{
	__asm__ __volatile__(
	    "movw		%1,%%dx\n\t"
	    "movw		%0,%%ax\n\t"
	    "outw		%%ax,%%dx\n\t"
	    ::"m"(data), "m"(port));
	return;
}

u32 io_read_port_dword(u32 port)
{
	u32 data;
	__asm__ __volatile__(
	    "movw		%1,%%dx\n\t"
	    "inl		%%dx,%%eax\n\t"
	    :"=%%eax"(data)
	    :"m"(port));
	return data;
}

void io_write_port_dword(u32 data, u32 port)
{
	__asm__ __volatile__(
	    "movw		%1,%%dx\n\t"
	    "movl		%0,%%eax\n\t"
	    "outl		%%eax,%%dx\n\t"
	    ::"m"(data), "m"(port));
	return;
}

void io_delay()
{
	__asm__ __volatile__(
	    "nop\n\t"
	    "nop\n\t"
	    "nop\n\t"
	    "nop\n\t"
	);
	return;
}
