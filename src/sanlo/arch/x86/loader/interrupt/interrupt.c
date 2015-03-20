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

#include "interrupt.h"
#include "../io/io.h"

void				io_delay();

static void			init_8259A();

void interrupt_init()
{
	init_8259A();
}

void init_8259A()
{
	//Master ICW1
	out_byte(0x11,0x20);
	io_delay();
	
	//Slave	ICW1
	out_byte(0x11,0xA0);
	io_delay();
	
	//Master ICW2
	//The interrupt number of IRQ starts from 0x20
	out_byte(0x20,0x21);
	io_delay();
	
	//Slave ICW2
	out_byte(0x28,0xA1);
	io_delay();
	
	//Master ICW3
	out_byte(0x04,0x21);
	io_delay();
	
	//Slave ICW3
	out_byte(0x02,0xA1);
	io_delay();
	
	//Master ICW4
	out_byte(0x01,0x21);
	io_delay();
	
	//Slave ICW4
	out_byte(0x01,0xA1);
	io_delay();
	
	//Master OCW1
	//Only Keyboard interrupt should be enabled
	out_byte(0xFD,0x21);	//0xFD=1111 1101
	io_delay();
	
	//Slave OCW1
	out_byte(0xFF,0x21);
	io_delay();
	
	return;
}