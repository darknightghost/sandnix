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

#include "../../../../../../../common/common.h"
#include "../../../../../debug/debug.h"
#include "../../../../port.h"

#define	IA32_APIC_BASE		0x001B
#define	LVT_LINT0			0x0350

u32		apic_base_addr;
u32		io_apic_base_addr;

void apic_init()
{
	dbg_kprint("Initializing APIC...\n");

	dbg_kprint("APIC base address : %P\n", apic_base_addr);
	dbg_kprint("I/O APIC base address : %P\n", io_apic_base_addr);

	//Disable 8259A
	__asm__ __volatile__(
	    "movl	(%0),%%eax\n"
	    "btsl	$16,%%eax\n"
	    "movl	%%eax,(%0)\n"
	    ::"b"(apic_base_addr+LVT_LINT0)
	    :"ax");

	dbg_kprint("8259A disabled.\n");

	//Enable APIC
	__asm__ __volatile__(
	    "rdmsr\n"
	    "btsl	$11,%%eax\n"
	    "wrmsr\n"
	    ::"c"(IA32_APIC_BASE)
	    :"ax", "dx");
}
