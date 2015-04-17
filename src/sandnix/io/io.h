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

#ifdef	X86

#include "../../common/arch/x86/types.h"
#include "arch/x86/int.h"

#endif	//X86

//Init

void		init_io();

//Ports
void		io_read_port_bytes(u32 port, u8* buf, u32 num);
void		io_write_port_bytes(u32 port, u8* buf, u32 num);

void		io_read_port_words(u32 port, u16* buf, u32 num);
void		io_write_port_words(u32 port, u16* buf, u32 num);

void		io_read_port_dwords(u32 port, u32* buf, u32 num);
void		io_write_port_dwords(u32 port, u32* buf, u32 num);

u8			io_read_port_byte(u32 port);
void		io_write_port_byte(u32 port, u8 data);

u16			io_read_port_word(u32 port);
void		io_write_port_word(u32 port, u16 data);

u32			io_read_port_dword(u32 port);
void		io_write_port_dword(u32 port, u32 data);

//Interrupts
bool		io_reg_int_hndlr();
void		io_unreg_int_hndlr();


void		io_set_int_lvl(u32 level);
u32			io_get_int_lvl();

#endif	//!	IO_H_INCLUDE
