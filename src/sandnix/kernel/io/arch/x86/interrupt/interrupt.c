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
#include "../../../../exceptions/exceptions.h"
#include "../../../../rtl/rtl.h"
#include "../../../../init/init.h"
#include "../../../interrupt.h"
#include "apic/apic.h"
#include "int_entry.h"

static	bool	is_apic_supported();
static	void	idt_init();

int_info_t	hndlr_table[256];

__asm__(".align	4\n");
idt_t		idt_table[256];

void interrupt_init()
{
	u32 i;
	dbg_kprint("Initializing interrupt controller...\n");

	for(i = 0; i < 255; i++) {
		if(i < 0x20) {
			hndlr_table[i].dealing_thrd = 0;
			hndlr_table[i].hndlr_entery = NULL;
			hndlr_table[i].priority = INT_PRIORITY_EXCEPTION;

		} else if(i <= IRQ15) {
			hndlr_table[i].dealing_thrd = 0;
			hndlr_table[i].hndlr_entery = NULL;
			hndlr_table[i].priority = INT_PRIORITY_IO;

		} else {
			hndlr_table[i].dealing_thrd = 0;
			hndlr_table[i].hndlr_entery = NULL;
			hndlr_table[i].priority = INT_PRIORITY_DISPATCH;
		}
	}

	idt_init();

	if(is_apic_supported()) {
		dbg_kprint("APIC supported...\n");
		apic_init();

	} else {
		excpt_panic(ENOCSI, "The hardware is too old.Sandnix requited APIC support.\n");
	}

	return;
}

void io_enable_interrupt()
{
	__asm__ __volatile__(
	    "sti\n"
	    :::);
	return;
}

void io_disable_interrupt()
{
	__asm__ __volatile__(
	    "cli\n"
	    :::);
	return;
}

bool is_apic_supported()
{
	bool ret;

	__asm__ __volatile__(
	    "movl	$1,%%eax\n"
	    "cpuid\n"
	    "bt		$9,%%edx\n"
	    "setcb	%0\n"
	    :"=a"(ret)
	    ::"bx", "cx", "dx");

	return ret;
}

void idt_init()
{
	idt_reg_t idt_t;
	u32 i;
	u32 address;

	dbg_kprint("Initializing IDT...\n");

	for(i = 0, address = (u32)int_0x00; i < 256; i++) {
		SET_NORMAL_IDT(idt_table, i, address);

		switch(i) {
			case 0x08:
			case 0x0B:
			case 0x0C:
			case 0x0D:
			case 0x0E:
			case 0x11:
				address += (u32)int_0x09 - (u32)int_0x08;
				break;

			default:
				address += (u32)int_0x01 - (u32)int_0x00;
		}
	}

	//Load IDT
	idt_t.base = (u32)idt_table;
	idt_t.limit = 256 * sizeof(idt_t) - 1;
	__asm__ __volatile__(
	    "lidt		%0\n"
	    ::"m"(idt_t));

	//Set IOPL
	__asm__ __volatile__(
	    "pushfl\n"
	    "movl	(%%esp),%%eax\n"
	    "andl	$0xFFFFCFFF,%%eax\n"
	    "movl	%%eax,(%%esp)\n"
	    "popfl\n"
	    :::"ax");
	return;
}

void int_dispatcher(u32 int_num, void* context,
                    u32 err_code)
{
	if(int_num < 0x20) {
		//Look for handler
		//Handler didn't found or the exception did not be handled
		excpt_panic(EINTERRUPT,
		            "Interrupt number = 0x%.2X\nError code = 0x%.8X\n",
		            int_num, err_code);

	} else {
		dbg_kprint("Interrupt 0x%.2X.\n", int_num);
		dbg_kprint("I/O APIC EOI register address : %p.\n", p_io_apic_eoi);
		int_ret(context);
	}
}

void int_ret(void* context)
{
	__asm__ __volatile__(
	    "movl	%0,%%esp\n"
	    "popal\n"
	    "iret\n"
	    ::"a"(context));
}
