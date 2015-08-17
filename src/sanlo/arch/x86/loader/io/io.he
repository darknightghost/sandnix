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

#ifndef	IO_H_INCLUDE
#define	IO_H_INCLUDE

#include "../types.h"

//Read port
u8		in_byte(u16 port);
u16		in_word(u16 port);

void	in_bytes(u16 port, u32 times, void* buf);
void	in_words(u16 port, u32 times, void* buf);

//Write port
void	out_byte(u8 data, u16 port);
void	out_word(u16 data, u16 port);

void	out_bytes(void* data, u16 port, u32 times);
void	out_words(void* data, u16 port, u32 times);

#endif	//! IO_H_INCLUDE
