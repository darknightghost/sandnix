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

//bool		int_hndlr_func(u32 int_num,u32 thread_id,u32 err_code);
typedef	bool	(*int_hndl_func)(u32, u32, u32);


typedef	struct _int_hndlr_info {
	struct 			_int_hndlr_info* p_next;
	int_hndl_func	func;
} int_hndlr_info_t, *pint_hndlr_info_t;

typedef	struct {
	u8				level;
	bool			called_flag;
	u32				err_code;
	u32				thread_id;
	pint_hndlr_info_t entry;
} int_hndlr_entry_t, *pint_hndlr_entry_t;

#pragma pack(1)

typedef	struct {
	u32		edi;
	u32		esi;
	u32		ebp;
	u32		esp;
	u32		ebx;
	u32		edx;
	u32		ecx;
	u32		eax;
} ret_regs_t, *pret_regs_t;
#pragma pack()

void		init_int_dispatcher();
void		int_excpt_dispatcher(u32 num, pret_regs_t p_regs);
void		int_normal_dispatcher(u32 num, pret_regs_t p_regs);
void		int_bp_dispatcher(pret_regs_t p_regs);
void		int_clock_dispatcher(pret_regs_t p_regs);

#endif	//!	INT_DISPATCHER_H_INCLUDE