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

#ifndef	INT_DISPATCHER_H_INCLUDE
#define	INT_DISPATCHER_H_INCLUDE

#include "../../io.h"
#include "../../../rtl/rtl.h"
#include "int_handler.h"
#include "../../../exceptions/exceptions.h"

#pragma pack(1)
typedef	struct _waiting_int {
	u32	level;
	struct {
		u32		edi;
		u32		esi;
		u32		ebp;
		u32		esp;
		u32		ebx;
		u32		edx;
		u32		ecx;
		u32		eax;
		u32		eflags;
	} regs;
} waiting_int, *pwaiting_int;

typedef	struct {
	u32		edi;
	u32		esi;
	u32		ebp;
	u32		esp;
	u32		ebx;
	u32		edx;
	u32		ecx;
	u32		eax;
	u32		eflags
} ret_regs, *pret_regs;
#pragma pack()

void		init_int_dispatcher();
void		int_excpt_dispatcher(u32 num, pret_regs p_regs);
void		int_normal_dispatcher(u32 num, pret_regs p_regs);
void		int_bp_dispatcher(pret_regs p_regs);

#endif	//!	INT_DISPATCHER_H_INCLUDE
