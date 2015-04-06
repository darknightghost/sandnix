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
#include "../string/string.h"

void		io_delay();
void		setup_8259A();
void		setup_idt();

void		int_00();
void		int_01();
void		int_02();
void		int_05();
void		int_06();
void		int_07();
void		int_08();
void		int_09();
void		int_0A();
void		int_0B();
void		int_0C();
void		int_0D();
void		int_0E();
void		int_0F();
void		int_10();
void		int_11();
void		int_12();
void		int_13();
void		int_21();

void setup_interrupt()
{
	setup_8259A();
	setup_idt();
	__asm__ __volatile__(
		"sti\n\t"
	);
	return;
}

void setup_8259A()
{
	//Master ICW1
	out_byte(0x11, 0x20);
	io_delay();
	//Slave	ICW1
	out_byte(0x11, 0xA0);
	io_delay();
	//Master ICW2
	//The interrupt number of IRQ starts from 0x20
	out_byte(0x20, 0x21);
	io_delay();
	//Slave ICW2
	out_byte(0x28, 0xA1);
	io_delay();
	//Master ICW3
	out_byte(0x04, 0x21);
	io_delay();
	//Slave ICW3
	out_byte(0x02, 0xA1);
	io_delay();
	//Master ICW4
	out_byte(0x01, 0x21);
	io_delay();
	//Slave ICW4
	out_byte(0x01, 0xA1);
	io_delay();
	//Master OCW1
	out_byte(0xFF, 0x21);
	io_delay();
	//Slave OCW1
	out_byte(0xFF, 0x21);
	io_delay();
	return;
}

void io_delay()
{
	__asm__ __volatile__(
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
		"nop\n\t"
	);
	return;
}

void setup_idt()
{
	idt_reg idt_value;

	memset((void*)IDT_BASE_ADDR, 0, INTERRUPT_MAX_NUM * sizeof(idt));
	SET_IDT(0x00, int_00, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x01, int_01, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x02, int_02, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x05, int_05, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x06, int_06, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x07, int_07, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x08, int_08, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x09, int_09, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x0A, int_0A, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x0B, int_0B, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x0C, int_0C, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x0D, int_0D, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x0E, int_0E, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x0F, int_0F, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x10, int_10, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x11, int_11, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x12, int_12, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(0x13, int_13, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);

	idt_value.base = IDT_BASE_ADDR;
	idt_value.limit = INTERRUPT_MAX_NUM * sizeof(idt) - 1;
	__asm__ __volatile__(
		"lidt		%0\n\t"
		::"m"(idt_value));
	return;
}
