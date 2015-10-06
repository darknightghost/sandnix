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

#include "ssddt.h"
#include "ssddt_funcs.h"
#include "../../rtl/rtl.h"
#include "../../io/io.h"
#include "../../pm/pm.h"
#include "../../exceptions/exceptions.h"
#include "../../debug/debug.h"
#include "../../mm/mm.h"

void ssddt_read_port(va_list p_args)
{
	//Agruments
	void* buf;
	u32 bits;
	u32 len;

	//Variables

	//Get args
	buf = va_arg(p_args, void*);
	bits = va_arg(p_args, u32);
	len = va_arg(p_args, u32);

	pm_set_errno(ESUCCESS);
	return;
}

void ssddt_write_port(va_list p_args)
{
	//Agruments
	void* buf;
	u32 bits;
	u32 len;

	//Variables

	//Get args
}

void ssddt_get_tickcount(va_list p_args)
{
	//Agruments
	u64* tick_count;

	//Variables

	//Get args
	tick_count = va_arg(p_args, u64*);

	//Check arguments
	if(!mm_virt_test(tick_count, sizeof(u64), PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	*tick_count = io_get_tick_count();

	pm_set_errno(ESUCCESS);

	return;
}

u32 ssddt_get_tick()
{
	pm_set_errno(ESUCCESS);
	return SYS_TICK;
}

k_status ssddt_set_int_msg(va_list p_args)
{
	//Agruments
	u32 int_num;

	//Variables

	//Get args
	int_num = va_arg(u32);

	if(io_int_msg_set(int_num)) {
		pm_set_errno(ESUCCESS);
		return ESUCCESS;

	} else {
		pm_set_errno(EINVAL);
		return EINVAL;
	}
}

void ssddt_clean_int_msg(va_list p_args)
{
	//Agruments
	u32 int_num;

	//Variables

	//Get args
	int_num = va_arg(u32);

	io_int_msg_clean(int_num);
	pm_set_errno(ESUCCESS);
	return;
}

void ssddt_kprint(va_list p_args)
{
	//Agruments
	char* fmt;
	va_list args;

	//Variables

	//Get args
	fmt = va_arg(p_args, char*);
	va_start(args, va_arg(p_args, char*));

	dbg_vprint(fmt_args);

	return;
}
