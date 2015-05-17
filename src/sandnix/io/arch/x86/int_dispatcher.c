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
#include "../../../rtl/rtl.h"
#include "int_handler.h"
#include "../../../exceptions/exceptions.h"

#define	HAS_ERR_CODE(num)	((num) == INT_DF)\
	|| (num) == INT_TS\
	|| (num) == INT_NP\
	|| (num) == INT_SS\
	|| (num) == INT_GP\
	|| (num) == INT_PF\
	|| (num) == INT_AC)

list				int_waiting_list = NULL;
int_hndlr_entry		int_hndlr_tbl[256];
bool				exception_handling_flag = false;
u32					int_reentry_count = 0;
u32					tick_count;
u8					current_int_level;

void init_int_dispatcher()
{
	rtl_memset(int_hndlr_tbl, 0, 256 * sizeof(int_hndlr_entry));
	return;
}

void int_excpt_dispatcher(u32 num, pret_regs p_regs)
{
	//Save thread context or interrupted interrupt service context
	if()

		//Increase reentry count
		int_reentry_count++;

	int_reentry_count--;
	__asm__ __volatitle__(
	    "sti\n\t"
	    ::);
	return;
}

void int_normal_dispatcher(u32 num, pret_regs p_regs)
{
	//Save thread context or interrupted interrupt service context
	//Increase reentry count
	int_reentry_count++;
	__asm__ __volatitle__(
	    "sti\n\t"
	    ::);
	return;
}

void int_bp_dispatcher(pret_regs p_regs)
{
	return;
}

bool io_reg_int_hndlr(u8 num, pint_hndlr_info p_info)
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

void io_unreg_int_hndlr(u8 num, pint_hndlr_info p_info)
{
	u8	current_lvl;
	pint_hndlr_info p_reged_info;

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
