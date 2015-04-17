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

static	idt			idt_table[256];
void*				int_hndlr_tbl[256];

static	void		setup_8259A();

void init_idt()
{
	u32 i;

	setup_8259A();

	rtl_memset(int_hndlr_tbl,0,256);

	//Enable interrupt
	__asm__ __volatile__(
		"sti\n\t"
		);
}

void setup_8259A()
{
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
	io_write_port_byte(0, 0x21);
	io_delay();

	//Slave OCW1
	io_write_port_byte(0, 0xA1);
	io_delay();
	return;
}