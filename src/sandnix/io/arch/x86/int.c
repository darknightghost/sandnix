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
int_hndlr_info		int_hndlr_tbl[256];

static	void		setup_8259A();
u32					tick_count;
u32					current_int_level;

void init_idt()
{
	u32 i;
	idt_reg idt;
	setup_8259A();
	rtl_memset(int_hndlr_tbl, 0, 256 * sizeof(int_hndlr_info));
	//Initialize IDT
	SET_IDT(idt_table, INT_DE, de_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_DB, db_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_NMI, nmi_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_BP, bp_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
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
	SET_IDT(idt_table, INT_RESERVED, default_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_MF, mf_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_AC, ac_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_MC, mc_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	SET_IDT(idt_table, INT_XF, xf_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);

	for(i = INT_XF + 1; i < 256; i++) {
		SET_IDT(idt_table, i, default_int_handler, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1);
	}

	//Load IDT
	idt.base = idt_table;
	idt.limit = 256 * sizeof(idt) - 1;
	__asm__ __volatile__(
		"lidt		%0\n\t"
		::"m"(idt));
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

bool io_reg_int_hndlr(u32 num, u8 level, void* entry)
{
	u32 crrnt_lvl;
	crrnt_lvl = io_get_int_lvl();

	if(crrnt_lvl > INT_LEVEL_DISPATCH) {
		//Panic
		return false;
	}

	io_set_int_lvl(INT_LEVEL_DISPATCH);

	if(int_hndlr_tbl[num].level != 0 || int_hndlr_tbl[num].func != NULL) {
		return false;
	}

	int_hndlr_tbl[num].level = level;
	int_hndlr_tbl[num].func = entry;
	io_set_int_lvl(crrnt_lvl);
	return true;
}

void io_unreg_int_hndlr(u32 num)
{
	u32 crrnt_lvl;
	crrnt_lvl = io_get_int_lvl();

	if(crrnt_lvl > INT_LEVEL_DISPATCH) {
		//Panic
		return false;
	}

	io_set_int_lvl(INT_LEVEL_DISPATCH);

	if(int_hndlr_tbl[num].level != 0 || int_hndlr_tbl[num].func != NULL) {
		int_hndlr_tbl[num].level = 0;
		int_hndlr_tbl[num].func = NULL;
	}

	io_set_int_lvl(crrnt_lvl);
	return;
}

void io_set_int_lvl(u32 level)
{
	current_int_level = level;
	return;
}

u32 io_get_int_lvl()
{
	return current_int_level;
}
