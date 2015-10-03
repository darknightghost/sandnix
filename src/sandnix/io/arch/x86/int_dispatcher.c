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
#include "../../../exceptions/exceptions.h"
#include "../../../rtl/rtl.h"
#include "int.h"
#include "int_handler.h"
#include "../../../debug/debug.h"
#include "../../../pm/pm.h"

#define	HAS_ERR_CODE(num)	((num) == INT_DF\
                             || (num) == INT_TS\
                             || (num) == INT_NP\
                             || (num) == INT_SS\
                             || (num) == INT_GP\
                             || (num) == INT_PF\
                             || (num) == INT_AC)

int_hndlr_entry_t		int_hndlr_tbl[256];
bool				exception_handling_flag;
u32					tick_count;
u8					current_int_level;
u32					new_int;
static	u32			dispatcher_thread = 0;

static	void		call_hndlr(u32 i);

void init_int_dispatcher()
{
	u32 i;

	exception_handling_flag = false;
	rtl_memset(int_hndlr_tbl, 0, 256 * sizeof(int_hndlr_entry_t));

	//Initialize int levels
	for(i = 0; i < 256; i++) {
		//Exceptions ,breakpoints and clocks
		//will be INT_LEVEL_EXCEPTION or INT_LEVEL_TASK.
		//Others will be INT_LEVEL_IO
		if(i <= INT_XF) {
			io_set_int_level(i, INT_LEVEL_EXCEPTION);

		} else {
			io_set_int_level(i, INT_LEVEL_IO);
		}

	}

	new_int = 0;

	return;
}

void int_excpt_dispatcher(u32 num, pret_regs_t p_regs)
{
	//Set interrupt status
	int_hndlr_tbl[num].called_flag = true;
	int_hndlr_tbl[num].thread_id = pm_get_crrnt_thrd_id();

	if(HAS_ERR_CODE(num)) {
		//Get error code
		int_hndlr_tbl[num].err_code = *(u32*)(p_regs + 1);

		//Move saved registers
		rtl_memmove(((u8*)(p_regs) + 4), p_regs, sizeof(ret_regs_t));

		p_regs = (pret_regs_t)((u8*)p_regs + 4);

	}

	if(new_int < int_hndlr_tbl[num].level) {
		new_int = int_hndlr_tbl[num].level;
	}

	//Resume interrupt dispatcher thread
	if(dispatcher_thread != 0) {
		pm_resume_thrd(dispatcher_thread);
	}

	//Schedule
	__asm__ __volatile__(
	    "leave\n\t"
	    "movl	%0,%%esp\n\t"
	    "call	pm_task_schedule\n\t"
	    ::"r"(p_regs));
	return;
}

void int_normal_dispatcher(u32 num, pret_regs_t p_regs)
{
	//Set interrupt status
	int_hndlr_tbl[num].called_flag = true;
	int_hndlr_tbl[num].thread_id = pm_get_crrnt_thrd_id();

	if(new_int < int_hndlr_tbl[INT_CLOCK].level) {
		new_int = int_hndlr_tbl[num].level;
	}

	//Resume interrupt dispatcher thread
	if(dispatcher_thread != 0) {
		pm_resume_thrd(dispatcher_thread);
	}

	//Schedule
	__asm__ __volatile__(
	    "leave\n\t"
	    "movl	%0,%%esp\n\t"
	    "call	pm_task_schedule\n\t"
	    ::"r"(p_regs));
	return;
}

void int_bp_dispatcher(pret_regs_t p_regs)
{
	//Set interrupt status
	int_hndlr_tbl[INT_BP].called_flag = true;
	int_hndlr_tbl[INT_BP].thread_id = pm_get_crrnt_thrd_id();

	//Resume interrupt dispatcher thread
	if(dispatcher_thread != 0) {
		pm_resume_thrd(dispatcher_thread);
	}

	if(new_int < int_hndlr_tbl[INT_CLOCK].level) {
		new_int = int_hndlr_tbl[INT_BP].level;
	}

	//Schedule
	__asm__ __volatile__(
	    "leave\n\t"
	    "movl	%0,%%esp\n\t"
	    "call	pm_task_schedule\n\t"
	    ::"r"(p_regs));
	return;
}

void int_clock_dispatcher(pret_regs_t p_regs)
{
	//Set interrupt status
	int_hndlr_tbl[INT_CLOCK].called_flag = true;
	int_hndlr_tbl[INT_CLOCK].thread_id = pm_get_crrnt_thrd_id();

	if(new_int < int_hndlr_tbl[INT_CLOCK].level) {
		new_int = int_hndlr_tbl[INT_CLOCK].level;
	}

	//Resume interrupt dispatcher thread
	if(int_hndlr_tbl[INT_CLOCK].entry != NULL) {
		if(dispatcher_thread != 0) {
			pm_resume_thrd(dispatcher_thread);
		}
	}


	//Enable next clock interrupt
	__asm__ __volatile__(
	    "movb	$0x20,%%al\n\t"
	    "outb	%%al,$0x20\n\t"
	    ::);

	//Schedule
	__asm__ __volatile__(
	    "leave\n\t"
	    "addl	$8,%%esp\n\t"
	    "call	pm_clock_schedule\n\t"
	    ::"r"(p_regs));
	return;
}

void io_dispatch_int(u32 thread_id, void* p_args)
{
	u32 i;

	io_set_crrnt_int_level(INT_LEVEL_EXCEPTION);

	dispatcher_thread = thread_id;

	while(1) {
		//Dispatch interrupts
		if(new_int >= INT_LEVEL_EXCEPTION) {

			for(i = 0; i < 256; i++) {
				//Exceptions must be handled,if not,call excpt_panic
				if(int_hndlr_tbl[i].called_flag
				   && int_hndlr_tbl[i].level == INT_LEVEL_EXCEPTION) {
					call_hndlr(i);
					int_hndlr_tbl[i].called_flag = false;
				}
			}
		}

		exception_handling_flag = false;

		for(i = 0; i < 256; i++) {
			if(int_hndlr_tbl[i].called_flag) {
				if(new_int > INT_LEVEL_IO) {
					continue;
				}

				call_hndlr(i);
				int_hndlr_tbl[i].called_flag = false;
			}
		}

		new_int = 0;
		io_set_crrnt_int_level(INT_LEVEL_EXCEPTION);
		pm_suspend_thrd(dispatcher_thread);
	}

	UNREFERRED_PARAMETER(p_args);
	return;
}

bool io_reg_int_hndlr(u8 num, pint_hndlr_info_t p_info)
{
	u8	current_lvl;

	if(p_info == NULL) {
		return false;
	}

	//Raise int level
	current_lvl = io_get_crrnt_int_level();

	if(current_lvl > INT_LEVEL_DISPATCH) {
		return false;
	}

	io_set_crrnt_int_level(INT_LEVEL_DISPATCH);

	//Regist interrupt handler
	if(num <= 0xFF) {
		p_info->p_next = int_hndlr_tbl[num].entry;
		int_hndlr_tbl[num].entry = p_info;

	} else {
		io_set_crrnt_int_level(current_lvl);
		return false;
	}

	//Low int level
	io_set_crrnt_int_level(current_lvl);

	return true;
}

void io_unreg_int_hndlr(u8 num, pint_hndlr_info_t p_info)
{
	u8	current_lvl;
	pint_hndlr_info_t p_reged_info;

	if(p_info == NULL) {
		return;
	}

	//Raise int level
	current_lvl = io_get_crrnt_int_level();

	if(current_lvl > INT_LEVEL_DISPATCH) {
		return;
	}

	io_set_crrnt_int_level(INT_LEVEL_DISPATCH);

	//Unregist interrupt handler
	if(int_hndlr_tbl[num].entry == p_info) {
		int_hndlr_tbl[num].entry = p_info->p_next;

	} else {
		for(p_reged_info = int_hndlr_tbl[num].entry;
		    p_reged_info != NULL;
		    p_reged_info = p_reged_info->p_next) {
			if(p_reged_info->p_next == p_info) {
				p_reged_info->p_next = p_info->p_next;
			}
		}
	}

	//Low int level
	io_set_crrnt_int_level(current_lvl);
	return;
}

void io_set_crrnt_int_level(u8 level)
{
	current_int_level = level;
	return;
}

u8 io_get_crrnt_int_level()
{
	return current_int_level;
}

u8 io_get_int_level(u8 num)
{
	return int_hndlr_tbl[num].level;
}

void io_set_int_level(u8 num, u8 level)
{
	if(num <= INT_XF) {
		int_hndlr_tbl[num].level = INT_LEVEL_EXCEPTION;

	} else {
		int_hndlr_tbl[num].level = level;
	}

	return;
}

u32 io_get_tick_count()
{
	return tick_count;
}

void call_hndlr(u32 i)
{
	pint_hndlr_info_t p_hndlr;
	u8	current_lvl;

	current_lvl = io_get_crrnt_int_level();

	//Check if handler exists
	if(int_hndlr_tbl[i].entry == NULL) {
		return;
	}

	//Call handlers
	p_hndlr = int_hndlr_tbl[i].entry;

	io_set_crrnt_int_level(current_lvl);

	do {
		if(p_hndlr->func(i,
		                 int_hndlr_tbl[i].thread_id,
		                 int_hndlr_tbl[i].err_code)) {

			io_set_crrnt_int_level(int_hndlr_tbl[i].level);
			return;
		}

		p_hndlr = p_hndlr->p_next;
	} while(p_hndlr != NULL);

	io_set_crrnt_int_level(current_lvl);

	return;
}
