/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

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

u8 hal_io_in_8(address_t port)
{
    u8 ret;

    __asm__ __volatile__(
        "inb	%1, %0\n"
        :"=a"(ret)
        :"d"((u16)port));
    return ret;
}
u16 hal_io_in_16(address_t port)
{
    u16 ret;

    __asm__ __volatile__(
        "inw	%1, %0\n"
        :"=a"(ret)
        :"d"((u16)port));
    return ret;
}

u32 hal_io_in_32(address_t port)
{
    u32 ret;

    __asm__ __volatile__(
        "inl	%1, %0\n"
        :"=a"(ret)
        :"d"((u16)port));
    return ret;
}

void hal_io_ins_8(void* dest, size_t count, address_t port)
{
    __asm__ __volatile__(
        "cld\n"
        "rep	insb\n"
        ::"D"(dest), "c"(count), "d"((u16)port)
        :"memory");
    return;
}

void hal_io_ins_16(void* dest, size_t count, address_t port)
{
    __asm__ __volatile__(
        "cld\n"
        "rep	insw\n"
        ::"D"(dest), "c"(count), "d"(port)
        :"memory");
    return;
}

void hal_io_ins_32(void* dest, size_t count, address_t port)
{
    __asm__ __volatile__(
        "cld\n"
        "rep	insl\n"
        ::"D"(dest), "c"(count), "d"(port)
        :"memory");
    return;
}

void hal_io_out_8(address_t port, u8 data)
{
    __asm__ __volatile__(
        "outb	%0, %1\n"
        ::"a"(data), "d"((u16)port));
    return;
}

void hal_io_out_16(address_t port, u16 data)
{
    __asm__ __volatile__(
        "outw	%0, %1\n"
        ::"a"(data), "d"((u16)port));
    return;
}

void hal_io_out_32(address_t port, u32 data)
{
    __asm__ __volatile__(
        "outl	%0, %1\n"
        ::"a"(data), "d"((u16)port));
    return;
}

void hal_io_outs_8(address_t port, size_t count, void* src)
{
    __asm__ __volatile__(
        "cld\n"
        "rep	outsb\n"
        ::"S"(src), "c"(count), "d"(port):);
    return;
}

void hal_io_outs_16(address_t port, size_t count, void* src)
{
    __asm__ __volatile__(
        "cld\n"
        "rep	outsw\n"
        ::"S"(src), "c"(count), "d"(port):);
    return;
}

void hal_io_outs_32(address_t port, size_t count, void* src)
{
    __asm__ __volatile__(
        "cld\n"
        "rep	outsl\n"
        ::"S"(src), "c"(count), "d"(port):);
    return;
}
