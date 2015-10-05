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
#include "../../../debug/debug.h"

static	idt_t			idt_table[256];
static	void		setup_8259A();
static	void		setup_clock();
extern	u32			tick_count;

void init_idt()
{
	idt_reg_t idt_t;

	dbg_print("%s", "Setting up 8259A...\n");
	setup_8259A();

	dbg_print("%s", "Setting up clock...\n");
	setup_clock();

	//Initialize IDT
	dbg_print("%s", "Initializing IDT...\n");
	SET_IDT(idt_table, INT_DE, de_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_DB, db_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_NMI, nmi_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_BP, bp_int_handler, SELECTOR_K_CODE, TYPE_TRAP, 0, 3, 1);
	SET_IDT(idt_table, INT_OF, of_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_BR, br_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_UD, ud_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_NM, nm_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_DF, df_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_FPU, fpu_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_TS, ts_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_NP, np_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_SS, ss_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_GP, gp_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_PF, pf_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_RESERVED, reserved_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_MF, mf_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_AC, ac_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_MC, mc_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_XF, xf_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);

	SET_NORMAL_IDT(idt_table, 0x14);
	SET_NORMAL_IDT(idt_table, 0x15);
	SET_NORMAL_IDT(idt_table, 0x16);
	SET_NORMAL_IDT(idt_table, 0x17);
	SET_NORMAL_IDT(idt_table, 0x18);
	SET_NORMAL_IDT(idt_table, 0x19);
	SET_NORMAL_IDT(idt_table, 0x1A);
	SET_NORMAL_IDT(idt_table, 0x1B);
	SET_NORMAL_IDT(idt_table, 0x1C);
	SET_NORMAL_IDT(idt_table, 0x1D);
	SET_NORMAL_IDT(idt_table, 0x1E);
	SET_NORMAL_IDT(idt_table, 0x1F);
	SET_NORMAL_IDT(idt_table, 0x20);
	SET_NORMAL_IDT(idt_table, 0x21);
	SET_NORMAL_IDT(idt_table, 0x22);
	SET_NORMAL_IDT(idt_table, 0x23);
	SET_NORMAL_IDT(idt_table, 0x24);
	SET_NORMAL_IDT(idt_table, 0x25);
	SET_NORMAL_IDT(idt_table, 0x26);
	SET_NORMAL_IDT(idt_table, 0x27);
	SET_NORMAL_IDT(idt_table, 0x28);
	SET_NORMAL_IDT(idt_table, 0x29);
	SET_NORMAL_IDT(idt_table, 0x2A);
	SET_NORMAL_IDT(idt_table, 0x2B);
	SET_NORMAL_IDT(idt_table, 0x2C);
	SET_NORMAL_IDT(idt_table, 0x2D);
	SET_NORMAL_IDT(idt_table, 0x2E);
	SET_NORMAL_IDT(idt_table, 0x2F);
	SET_NORMAL_IDT(idt_table, 0x30);
	SET_NORMAL_IDT(idt_table, 0x31);
	SET_NORMAL_IDT(idt_table, 0x32);
	SET_NORMAL_IDT(idt_table, 0x33);
	SET_NORMAL_IDT(idt_table, 0x34);
	SET_NORMAL_IDT(idt_table, 0x35);
	SET_NORMAL_IDT(idt_table, 0x36);
	SET_NORMAL_IDT(idt_table, 0x37);
	SET_NORMAL_IDT(idt_table, 0x38);
	SET_NORMAL_IDT(idt_table, 0x39);
	SET_NORMAL_IDT(idt_table, 0x3A);
	SET_NORMAL_IDT(idt_table, 0x3B);
	SET_NORMAL_IDT(idt_table, 0x3C);
	SET_NORMAL_IDT(idt_table, 0x3D);
	SET_NORMAL_IDT(idt_table, 0x3E);
	SET_NORMAL_IDT(idt_table, 0x3F);
	SET_NORMAL_IDT(idt_table, 0x40);
	SET_NORMAL_IDT(idt_table, 0x41);
	SET_NORMAL_IDT(idt_table, 0x42);
	SET_NORMAL_IDT(idt_table, 0x43);
	SET_NORMAL_IDT(idt_table, 0x44);
	SET_NORMAL_IDT(idt_table, 0x45);
	SET_NORMAL_IDT(idt_table, 0x46);
	SET_NORMAL_IDT(idt_table, 0x47);
	SET_NORMAL_IDT(idt_table, 0x48);
	SET_NORMAL_IDT(idt_table, 0x49);
	SET_NORMAL_IDT(idt_table, 0x4A);
	SET_NORMAL_IDT(idt_table, 0x4B);
	SET_NORMAL_IDT(idt_table, 0x4C);
	SET_NORMAL_IDT(idt_table, 0x4D);
	SET_NORMAL_IDT(idt_table, 0x4E);
	SET_NORMAL_IDT(idt_table, 0x4F);
	SET_NORMAL_IDT(idt_table, 0x50);
	SET_NORMAL_IDT(idt_table, 0x51);
	SET_NORMAL_IDT(idt_table, 0x52);
	SET_NORMAL_IDT(idt_table, 0x53);
	SET_NORMAL_IDT(idt_table, 0x54);
	SET_NORMAL_IDT(idt_table, 0x55);
	SET_NORMAL_IDT(idt_table, 0x56);
	SET_NORMAL_IDT(idt_table, 0x57);
	SET_NORMAL_IDT(idt_table, 0x58);
	SET_NORMAL_IDT(idt_table, 0x59);
	SET_NORMAL_IDT(idt_table, 0x5A);
	SET_NORMAL_IDT(idt_table, 0x5B);
	SET_NORMAL_IDT(idt_table, 0x5C);
	SET_NORMAL_IDT(idt_table, 0x5D);
	SET_NORMAL_IDT(idt_table, 0x5E);
	SET_NORMAL_IDT(idt_table, 0x5F);
	SET_NORMAL_IDT(idt_table, 0x60);
	SET_NORMAL_IDT(idt_table, 0x61);
	SET_NORMAL_IDT(idt_table, 0x62);
	SET_NORMAL_IDT(idt_table, 0x63);
	SET_NORMAL_IDT(idt_table, 0x64);
	SET_NORMAL_IDT(idt_table, 0x65);
	SET_NORMAL_IDT(idt_table, 0x66);
	SET_NORMAL_IDT(idt_table, 0x67);
	SET_NORMAL_IDT(idt_table, 0x68);
	SET_NORMAL_IDT(idt_table, 0x69);
	SET_NORMAL_IDT(idt_table, 0x6A);
	SET_NORMAL_IDT(idt_table, 0x6B);
	SET_NORMAL_IDT(idt_table, 0x6C);
	SET_NORMAL_IDT(idt_table, 0x6D);
	SET_NORMAL_IDT(idt_table, 0x6E);
	SET_NORMAL_IDT(idt_table, 0x6F);
	SET_NORMAL_IDT(idt_table, 0x70);
	SET_NORMAL_IDT(idt_table, 0x71);
	SET_NORMAL_IDT(idt_table, 0x72);
	SET_NORMAL_IDT(idt_table, 0x73);
	SET_NORMAL_IDT(idt_table, 0x74);
	SET_NORMAL_IDT(idt_table, 0x75);
	SET_NORMAL_IDT(idt_table, 0x76);
	SET_NORMAL_IDT(idt_table, 0x77);
	SET_NORMAL_IDT(idt_table, 0x78);
	SET_NORMAL_IDT(idt_table, 0x79);
	SET_NORMAL_IDT(idt_table, 0x7A);
	SET_NORMAL_IDT(idt_table, 0x7B);
	SET_NORMAL_IDT(idt_table, 0x7C);
	SET_NORMAL_IDT(idt_table, 0x7D);
	SET_NORMAL_IDT(idt_table, 0x7E);
	SET_NORMAL_IDT(idt_table, 0x7F);
	SET_NORMAL_IDT(idt_table, 0x80);
	SET_NORMAL_IDT(idt_table, 0x81);
	SET_NORMAL_IDT(idt_table, 0x82);
	SET_NORMAL_IDT(idt_table, 0x83);
	SET_NORMAL_IDT(idt_table, 0x84);
	SET_NORMAL_IDT(idt_table, 0x85);
	SET_NORMAL_IDT(idt_table, 0x86);
	SET_NORMAL_IDT(idt_table, 0x87);
	SET_NORMAL_IDT(idt_table, 0x88);
	SET_NORMAL_IDT(idt_table, 0x89);
	SET_NORMAL_IDT(idt_table, 0x8A);
	SET_NORMAL_IDT(idt_table, 0x8B);
	SET_NORMAL_IDT(idt_table, 0x8C);
	SET_NORMAL_IDT(idt_table, 0x8D);
	SET_NORMAL_IDT(idt_table, 0x8E);
	SET_NORMAL_IDT(idt_table, 0x8F);
	SET_NORMAL_IDT(idt_table, 0x90);
	SET_NORMAL_IDT(idt_table, 0x91);
	SET_NORMAL_IDT(idt_table, 0x92);
	SET_NORMAL_IDT(idt_table, 0x93);
	SET_NORMAL_IDT(idt_table, 0x94);
	SET_NORMAL_IDT(idt_table, 0x95);
	SET_NORMAL_IDT(idt_table, 0x96);
	SET_NORMAL_IDT(idt_table, 0x97);
	SET_NORMAL_IDT(idt_table, 0x98);
	SET_NORMAL_IDT(idt_table, 0x99);
	SET_NORMAL_IDT(idt_table, 0x9A);
	SET_NORMAL_IDT(idt_table, 0x9B);
	SET_NORMAL_IDT(idt_table, 0x9C);
	SET_NORMAL_IDT(idt_table, 0x9D);
	SET_NORMAL_IDT(idt_table, 0x9E);
	SET_NORMAL_IDT(idt_table, 0x9F);
	SET_NORMAL_IDT(idt_table, 0xA0);
	SET_NORMAL_IDT(idt_table, 0xA1);
	SET_NORMAL_IDT(idt_table, 0xA2);
	SET_NORMAL_IDT(idt_table, 0xA3);
	SET_NORMAL_IDT(idt_table, 0xA4);
	SET_NORMAL_IDT(idt_table, 0xA5);
	SET_NORMAL_IDT(idt_table, 0xA6);
	SET_NORMAL_IDT(idt_table, 0xA7);
	SET_NORMAL_IDT(idt_table, 0xA8);
	SET_NORMAL_IDT(idt_table, 0xA9);
	SET_NORMAL_IDT(idt_table, 0xAA);
	SET_NORMAL_IDT(idt_table, 0xAB);
	SET_NORMAL_IDT(idt_table, 0xAC);
	SET_NORMAL_IDT(idt_table, 0xAD);
	SET_NORMAL_IDT(idt_table, 0xAE);
	SET_NORMAL_IDT(idt_table, 0xAF);
	SET_NORMAL_IDT(idt_table, 0xB0);
	SET_NORMAL_IDT(idt_table, 0xB1);
	SET_NORMAL_IDT(idt_table, 0xB2);
	SET_NORMAL_IDT(idt_table, 0xB3);
	SET_NORMAL_IDT(idt_table, 0xB4);
	SET_NORMAL_IDT(idt_table, 0xB5);
	SET_NORMAL_IDT(idt_table, 0xB6);
	SET_NORMAL_IDT(idt_table, 0xB7);
	SET_NORMAL_IDT(idt_table, 0xB8);
	SET_NORMAL_IDT(idt_table, 0xB9);
	SET_NORMAL_IDT(idt_table, 0xBA);
	SET_NORMAL_IDT(idt_table, 0xBB);
	SET_NORMAL_IDT(idt_table, 0xBC);
	SET_NORMAL_IDT(idt_table, 0xBD);
	SET_NORMAL_IDT(idt_table, 0xBE);
	SET_NORMAL_IDT(idt_table, 0xBF);
	SET_NORMAL_IDT(idt_table, 0xC0);
	SET_NORMAL_IDT(idt_table, 0xC1);
	SET_NORMAL_IDT(idt_table, 0xC2);
	SET_NORMAL_IDT(idt_table, 0xC3);
	SET_NORMAL_IDT(idt_table, 0xC4);
	SET_NORMAL_IDT(idt_table, 0xC5);
	SET_NORMAL_IDT(idt_table, 0xC6);
	SET_NORMAL_IDT(idt_table, 0xC7);
	SET_NORMAL_IDT(idt_table, 0xC8);
	SET_NORMAL_IDT(idt_table, 0xC9);
	SET_NORMAL_IDT(idt_table, 0xCA);
	SET_NORMAL_IDT(idt_table, 0xCB);
	SET_NORMAL_IDT(idt_table, 0xCC);
	SET_NORMAL_IDT(idt_table, 0xCD);
	SET_NORMAL_IDT(idt_table, 0xCE);
	SET_NORMAL_IDT(idt_table, 0xCF);
	SET_NORMAL_IDT(idt_table, 0xD0);
	SET_NORMAL_IDT(idt_table, 0xD1);
	SET_NORMAL_IDT(idt_table, 0xD2);
	SET_NORMAL_IDT(idt_table, 0xD3);
	SET_NORMAL_IDT(idt_table, 0xD4);
	SET_NORMAL_IDT(idt_table, 0xD5);
	SET_NORMAL_IDT(idt_table, 0xD6);
	SET_NORMAL_IDT(idt_table, 0xD7);
	SET_NORMAL_IDT(idt_table, 0xD8);
	SET_NORMAL_IDT(idt_table, 0xD9);
	SET_NORMAL_IDT(idt_table, 0xDA);
	SET_NORMAL_IDT(idt_table, 0xDB);
	SET_NORMAL_IDT(idt_table, 0xDC);
	SET_NORMAL_IDT(idt_table, 0xDD);
	SET_NORMAL_IDT(idt_table, 0xDE);
	SET_NORMAL_IDT(idt_table, 0xDF);
	SET_NORMAL_IDT(idt_table, 0xE0);
	SET_NORMAL_IDT(idt_table, 0xE1);
	SET_NORMAL_IDT(idt_table, 0xE2);
	SET_NORMAL_IDT(idt_table, 0xE3);
	SET_NORMAL_IDT(idt_table, 0xE4);
	SET_NORMAL_IDT(idt_table, 0xE5);
	SET_NORMAL_IDT(idt_table, 0xE6);
	SET_NORMAL_IDT(idt_table, 0xE7);
	SET_NORMAL_IDT(idt_table, 0xE8);
	SET_NORMAL_IDT(idt_table, 0xE9);
	SET_NORMAL_IDT(idt_table, 0xEA);
	SET_NORMAL_IDT(idt_table, 0xEB);
	SET_NORMAL_IDT(idt_table, 0xEC);
	SET_NORMAL_IDT(idt_table, 0xED);
	SET_NORMAL_IDT(idt_table, 0xEE);
	SET_NORMAL_IDT(idt_table, 0xEF);
	SET_NORMAL_IDT(idt_table, 0xF0);
	SET_NORMAL_IDT(idt_table, 0xF1);
	SET_NORMAL_IDT(idt_table, 0xF2);
	SET_NORMAL_IDT(idt_table, 0xF3);
	SET_NORMAL_IDT(idt_table, 0xF4);
	SET_NORMAL_IDT(idt_table, 0xF5);
	SET_NORMAL_IDT(idt_table, 0xF6);
	SET_NORMAL_IDT(idt_table, 0xF7);
	SET_NORMAL_IDT(idt_table, 0xF8);
	SET_NORMAL_IDT(idt_table, 0xF9);
	SET_NORMAL_IDT(idt_table, 0xFA);
	SET_NORMAL_IDT(idt_table, 0xFB);
	SET_NORMAL_IDT(idt_table, 0xFC);
	SET_NORMAL_IDT(idt_table, 0xFD);
	SET_NORMAL_IDT(idt_table, 0xFE);
	SET_NORMAL_IDT(idt_table, 0xFF);

	//Load IDT
	idt_t.base = (u32)idt_table;
	idt_t.limit = 256 * sizeof(idt_t) - 1;
	__asm__ __volatile__(
	    "lidt		%0\n\t"
	    ::"m"(idt_t));

	//Set IOPL
	__asm__ __volatile__(
	    "pushfl\n\t"
	    "movl	(%%esp),%%eax\n\t"
	    "andl	$0xFFFFCFFF,%%eax\n\t"
	    "movl	%%eax,(%%esp)\n\t"
	    "popfl\n\t"
	    :::"ax");
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
	dbg_print("%s", "IRQ0	=>	INT 0x20\n");

	//Slave ICW2
	io_write_port_byte(0x28, 0xA1);
	io_delay();
	dbg_print("%s", "IRQ8	=>	INT 0x28\n");

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

void io_enable_interrupt()
{
	__asm__ __volatile__(
	    "sti\n\t"
	);
	return;
}

void io_disable_interrupt()
{
	__asm__ __volatile__(
	    "cli\n\t"
	);
	return;
}
