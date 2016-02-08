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


#include "../../../../../../common/common.h"
#include "../../../../debug/debug.h"

#define	INT_8359A		0
#define	INT_APIC		1

static	bool	is_apic_supported();
static	u32		int_controller;

void interrupt_init()
{
	dbg_kprint("Initializing interrupt controller...\n");

	if(is_apic_supported()) {
		dbg_kprint("APIC supported...\n");
		int_controller = INT_APIC;

	} else {
		dbg_kprint("APIC not supported...\n");
		int_controller = INT_8359A;
	}

	return;
}

void		io_enable_interrupt();
void		io_disable_interrupt();


bool is_apic_supported()
{
	bool ret;

	__asm__ __volatile__(
	    "movl	$1,%%eax\n"
	    "cpuid\n"
	    "bt		$21,%%ecx\n"
	    "setcb	%0\n"
	    :"=a"(ret)
	    ::);

	return ret;
}
