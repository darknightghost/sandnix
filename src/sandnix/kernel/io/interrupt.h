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
#include "port.h"

#ifdef	X86
	#include "arch/x86/interrupt/interrupt.h"
#endif	//	X86

//Interrupt level
#define		INT_PRIORITY_HIGHEST		0xFF
#define		INT_PRIORITY_LOWEST			0x00

#define		INT_PRIORITY_EXCEPTION		0xFF
#define		INT_PRIORITY_IO				0xD0
#define		INT_PRIORITY_DISPATCH		0x40	//Dispatch messages and signals,the task will not switch.
#define		INT_PRIORITY_CLOCK			0x30
#define		INT_PRIORITY_USR_HIGHEST	0x0F
#define		INT_PRIORITY_IDLE			0x00

typedef	struct	_int_hndlr_info_t {
	struct	_int_hndlr_info_t*	p_next;
	void*	hndlr;
} int_hndlr_t, *pint_hndlr_info_t;

typedef	struct	_int_info_t {
	u32					priority;
	u32					dealing_thrd;
	pint_hndlr_info_t	hndlr_entery;
} int_info_t, *pint_info_t;

void		interrupt_init();

void		io_enable_interrupt();
void		io_disable_interrupt();

void		io_disable_all_interrupt();

void		io_regist_int_hndlr(u32 int_num, pint_hndlr_info_t p_hndlr);
void		io_unregist_int_hndlr(u32 int_num, pint_hndlr_info_t p_hndlt);

void		io_set_interrupt_priority(u32 int_num, u32 priority);
u32			io_get_interrupt_priority(u32 int_num);


u32			io_get_sys_ticks();
//If you use this function to deal interrupts,the context of function is unsure
//and the io module will not deal with this interrupt.
//The task-switching function use it to hook system clock.
void		io_set_int_entry(u32 int_num, void* entry);
void*		io_clear_int_entry(u32 int_num);
