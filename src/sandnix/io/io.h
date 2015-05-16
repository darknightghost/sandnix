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
	#include "arch/x86/int_dispatcher.h"

#endif	//X86

//Interrupt level
#define		INT_LEVEL_HIGHEST		0xFF
#define		INT_LEVEL_LOWEST		0x00

#define		INT_LEVEL_EXCEPTION		0xFF
#define		INT_LEVEL_IO			0xD0
#define		INT_LEVEL_CLOCK			0xB0
#define		INT_LEVEL_DISPATCH		0x40	//Dispatch messages and signals,the task will not switch.
#define		INT_LEVEL_TASK			0x20
#define		INT_LEVEL_USR_HIGHEST	0x0F
#define		INT_LEVEL_IDLE			0x00

//System time
#define		SYS_TICK				10

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
void		io_write_port_byte(u8 data, u32 port);

u16			io_read_port_word(u32 port);
void		io_write_port_word(u16 data, u32 port);

u32			io_read_port_dword(u32 port);
void		io_write_port_dword(u32 data, u32 port);

void		io_delay();

//Interrupts
//These two functions can only be called when Interrupt level <= INT_LEVEL_DISPATCH
bool		io_reg_int_hndlr(u8 num, pint_hndlr_info p_info);
void		io_unreg_int_hndlr(u8 num, pint_hndlr_info p_info);

u8			io_get_int_level(u8 num);
void		io_set_int_level(u8 num, u8 level);

void		io_set_crrnt_int_level(u8 level);
u8			io_get_crrnt_int_level();

u32			io_get_tick_count();

void		io_enable_interrupt();
void		io_disable_interrupt();

#endif	//!	IO_H_INCLUDE
