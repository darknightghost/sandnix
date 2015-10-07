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
	u32 port;
	void* buf;
	size_t bits;

	//Variables

	//Get args
	port = va_arg(p_args, u32);
	buf = va_arg(p_args, void*);
	bits = va_arg(p_args, size_t);

	//Check arguments
	if(port > 255) {
		pm_set_errno(EINVAL);
		return;
	}

	if(!mm_virt_test(buf, bit, PG_STAT_WRITEABLE | PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	switch(bits) {
	case 1:
		*(u8*)buf = io_read_port_byte(port);
		break;

	case 2:
		*(u16*)buf = io_read_port_word(port);
		break;

	case 4:
		*(u32*)buf = io_read_port_dword(port);
		break;

	default:
		pm_set_errno(EINVAL);
		return;
	}

	pm_set_errno(ESUCCESS);
	return;
}

void ssddt_write_port(va_list p_args)
{
	//Agruments
	u32 port;
	void* buf;
	size_t bits;

	//Variables

	//Get args
	port = va_arg(p_args, u32);
	buf = va_arg(p_args, void*);
	bits = va_arg(p_args, size_t);

	//Check arguments
	if(port > 255) {
		pm_set_errno(EINVAL);
		return;
	}

	if(!mm_virt_test(buf, bit, PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	switch(bits) {
	case 1:
		io_write_port_byte(*(u8*)buf, port);
		break;

	case 2:
		io_write_port_word(*(u16*)buf, port);
		break;

	case 4:
		io_write_port_dword(*(u32*)buf, port);
		break;

	default:
		pm_set_errno(EINVAL);
		return;
	}

	pm_set_errno(ESUCCESS);
	return;
}

void ssddt_read_port_datas(va_list p_args)
{
	//Agruments
	u32 port;
	void* buf;
	size_t bits;
	size_t len;

	//Variables

	//Get args
	port = va_arg(p_args, u32);
	buf = va_arg(p_args, void*);
	bits = va_arg(p_args, size_t);
	len = va_arg(p_args, size_t);

	//Check arguments
	if(port > 255) {
		pm_set_errno(EINVAL);
		return;
	}

	if(!mm_virt_test(buf, len, PG_STAT_WRITEABLE | PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	switch(bits) {
	case 1:
		io_read_port_bytes(port, buf, len);
		break;

	case 2:
		io_read_port_words(port, buf, len / bits);
		break;

	case 4:
		io_read_port_dwords(port, buf, len / bits);
		break;

	default:
		pm_set_errno(EINVAL);
		return;
	}

	pm_set_errno(ESUCCESS);
	return;
}

void ssddt_write_port_datas(va_list p_args)
{
	//Agruments
	u32 port;
	void* buf;
	size_t bits;
	size_t len;

	//Variables

	//Get args
	port = va_arg(p_args, u32);
	buf = va_arg(p_args, void*);
	bits = va_arg(p_args, size_t);
	len = va_arg(p_args, size_t);

	//Check arguments
	if(port > 255) {
		pm_set_errno(EINVAL);
		return;
	}

	if(!mm_virt_test(buf, len, PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	switch(bits) {
	case 1:
		io_write_port_bytes(port, buf, len);
		break;

	case 2:
		io_write_port_words(port, buf, len / bits);
		break;

	case 4:
		io_write_port_dwords(port, buf, len / bits);
		break;

	default:
		pm_set_errno(EINVAL);
		return;
	}

	pm_set_errno(ESUCCESS);
	return;
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
