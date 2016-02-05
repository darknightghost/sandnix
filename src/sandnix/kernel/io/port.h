/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#pragma once

#include "../../../common/common.h"

void		io_read_port_bytes(u32 port, u8* buf, u32 num);
void		io_write_port_bytes(u32 port, u8* buf, u32 num);

void		io_read_port_words(u32 port, u16* buf, u32 num);
void		io_write_port_words(u32 port, u16* buf, u32 num);

void		io_read_port_dwords(u32 port, u32* buf, u32 num);
void		io_write_port_dwords(u32 port, u32* buf, u32 num);

u8			io_read_port_byte(u32 port);
void		io_write_port_byte(u8 data, u32 port);

u16			io_read_port_word(u32 port);
void		io_write_port_word(u16 data, u32 port);

u32			io_read_port_dword(u32 port);
void		io_write_port_dword(u32 data, u32 port);

void		io_delay();
