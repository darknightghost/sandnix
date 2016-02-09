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

static	void	disable_8259a();

void apic_init()
{
	u32	apic_phy_addr;

	dbg_kprint("Initializing APIC...\n");
	disable_8259a();

	//Enable APIC
	__asm__ __volatile__(
	    "rdmsr\n"
	    "btsl	$11,%%eax\n"
	    "wrmsr\n"
	    "andl	$0xfffff000,%%eax\n"
	    :"=a"(apic_phy_addr)
	    :"c"(IA32_APIC_BASE)
	    :"dx");
	dbg_kprint("APIC physical address:%P\n", apic_phy_addr);
}


void disable_8259a()
{
	dbg_kprint("Disabling 8259A...\n");
	//Master ICW1
	io_write_port_byte(0x11, 0x20);
	io_delay();

	//Slave	ICW1
	io_write_port_byte(0x11, 0xA0);
	io_delay();

	//Master ICW2
	io_write_port_byte(0x20, 0x21);
	io_delay();

	//Slave ICW2
	io_write_port_byte(0x28, 0xA1);
	io_delay();

	//Master ICW3
	io_write_port_byte(0x04, 0x21);
	io_delay();

	//Slave ICW3
	io_write_port_byte(0x02, 0xA1);
	io_delay();

	//Master ICW4
	io_write_port_byte(0x01, 0x21);
	io_delay();

	//Slave ICW4
	io_write_port_byte(0x01, 0xA1);
	io_delay();

	//Master OCW1
	io_write_port_byte(0xFF, 0x21);
	io_delay();

	//Slave OCW1
	io_write_port_byte(0xFF, 0xA1);
	io_delay();
	return;
}
