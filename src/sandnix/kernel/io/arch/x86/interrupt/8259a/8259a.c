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
#include "../../../../../rtl/rtl.h"
#include "../../../../io.h"
#include "../../../../port.h"
#include "../interrupt.h"
#include "8259a.h"

static	void	setup_clock();

void _8259a_init()
{
	dbg_kprint("Initializing 8259A...\n");

	//Master ICW1
	io_write_port_byte(0x11, 0x20);
	io_delay();

	//Slave	ICW1
	io_write_port_byte(0x11, 0xA0);
	io_delay();

	//Master ICW2
	//The interrupt number of IRQ starts from 0x20
	io_write_port_byte(0x20, 0x21);
	io_delay();
	dbg_kprint("%s", "IRQ0	=>	INT 0x20\n");

	//Slave ICW2
	io_write_port_byte(0x28, 0xA1);
	io_delay();
	dbg_kprint("%s", "IRQ8	=>	INT 0x28\n");

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
	io_write_port_byte(0, 0x21);
	io_delay();

	//Slave OCW1
	io_write_port_byte(0, 0xA1);
	io_delay();

	setup_clock();
	return;
}

void setup_clock()
{
	u16	count0;

	tick_count = 0;

	count0 = 1193180 / (1000 / SYS_TICK);

	io_write_port_byte(0x34, 0x43);

	io_write_port_byte(count0 & 0xFF, 0x40);
	io_write_port_byte((count0 >> 8) & 0xFF, 0x40);

	return;
}

void _8259a_send_eoi()
{
	io_write_port_byte(0x20, 0x20);
	return;
}
