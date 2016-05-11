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
#include "apic.h"

#define	IA32_APIC_BASE			0x001B
#define	LVT_LINT0				0x0350
#define	LVT_TIMER				0x0320
#define	TIMER_INITIAL_COUNT_REG	0x0380
#define	TIMER_CURRENT_COUNT_REG	0x0390
#define	TIMER_DIVIDE_CONF_REG	0x03E0

#define	GET_RTC_SEC()\
	({ \
		u8 __tmp_sec; \
		__asm__ __volatile__( \
		                      "xorl		%%eax,%%eax\n" \
		                      "outb		%%al,$0x70\n" \
		                      "inb		$0x71,%%al\n" \
		                      :"=a"(__tmp_sec) \
		                      ::"bx","cx","dx"); \
		(u8)((__tmp_sec & 0x0F) + (__tmp_sec >> 4) * 10); \
	})

#define	SEC_MINUS(a,b) \
	({ \
		u8 __tmp_ret; \
		if((a) < (b)) { \
			__tmp_ret = (a) + 60 - (b); \
		} else { \
			__tmp_ret = (a) - (b); \
		} \
		__tmp_ret; \
	})

#define	READ_TSC() \
	({ \
		u32	__tmp_time; \
		__asm__ __volatile__(\
		                     "rdtsc\n" \
		                     :"=a"(__tmp_time) \
		                     ::"dx"); \
		__tmp_time; \
	})

u32		apic_base_addr;
u32		io_apic_base_addr;
u32*	p_io_apic_index;
u32*	p_io_apic_data;
u32*	p_io_apic_eoi;

u32		bsp_cpu_freq;

static	void	clock_init();
static	u32		get_init_count();

void apic_init()
{
	u32 apic_id;
	u32 index;

	dbg_kprint("Initializing APIC...\n");

	dbg_kprint("APIC base address : %P\n", apic_base_addr);
	dbg_kprint("I/O APIC base address : %P\n", io_apic_base_addr);

	//Disable 8259A
	*(u32*)(apic_base_addr + LVT_LINT0) |= 0x10000;
	io_write_port_byte(0xFF, 0x21);

	dbg_kprint("8259A disabled.\n");

	//Enable local APIC
	__asm__ __volatile__(
	    "rdmsr\n"
	    "btsl	$11,%%eax\n"
	    "wrmsr\n"
	    ::"c"(IA32_APIC_BASE)
	    :"ax", "dx");

	//Get current APIC ID
	apic_id = *(u32*)(apic_base_addr + 0x20);
	dbg_kprint("BSP CPU APIC ID : %P.\n", apic_id);

	//I/O APIC enabled in init/arch/x86/init.S
	p_io_apic_index = (u32*)(io_apic_base_addr);
	p_io_apic_data = (u32*)(io_apic_base_addr + 0x10);
	p_io_apic_eoi = (u32*)(io_apic_base_addr + 0x40);
	dbg_kprint("I/O APIC Index register address : %p.\n", p_io_apic_index);
	dbg_kprint("I/O APIC Data register address : %p.\n", p_io_apic_data);
	dbg_kprint("I/O APIC EOI register address : %p.\n", p_io_apic_eoi);

	//IRQ0 is not enabled.System clock will use it
	//for(index = 1; index < 23; index++) {
	for(index = 1; index < 17; index++) {
		*p_io_apic_index = IRQ0_INDEX + index * 2;
		io_delay();
		__asm__ __volatile__("":::"memory");
		*p_io_apic_data = IRQ0 + index;
		io_delay();
		*p_io_apic_index = IRQ0_INDEX + index * 2 + 1;
		io_delay();
		__asm__ __volatile__("":::"memory");
		*p_io_apic_data = apic_id;
		io_delay();
		dbg_kprint("IRQ%u ==> INT 0x%.2X.\n", index, IRQ0 + index);
	}

	__asm__ __volatile__("":::"memory");
	*p_io_apic_eoi = 0;

	//Enable system clock
	clock_init();

	return;
}

void clock_init()
{
	u32 init_count;

	u32* p_timer_reg;
	u32* p_init_count_reg;
	u32* p_current_count_reg;
	u32* p_divide_conf_reg;

	dbg_kprint("Initializing system clock...\n");

	p_timer_reg = (u32*)(apic_base_addr + LVT_TIMER);
	p_init_count_reg = (u32*)(apic_base_addr + TIMER_INITIAL_COUNT_REG);
	p_current_count_reg = (u32*)(apic_base_addr + TIMER_CURRENT_COUNT_REG);
	p_divide_conf_reg = (u32*)(apic_base_addr + TIMER_DIVIDE_CONF_REG);

	u32 i;

	for(i = 0; i < 100; i++) {
		dbg_kprint("%P\n", get_init_count());
	}

	init_count = get_init_count();
	*p_init_count_reg = init_count;
	*p_current_count_reg = init_count;
	dbg_kprint("%d clock cycles per tick.\n", init_count);

	*p_divide_conf_reg |= 0x0000000B;
	//*p_timer_reg = INT_CLOCK | 0x20000;
	UNREFERRED_PARAMETER(p_timer_reg);
}

u32 get_init_count()
{
	u32 tsc1, tsc2;
	u8 second1;
	u8 second2;

	//Get the number of clock cycles per second
	second1 = GET_RTC_SEC();

	while(second1 == GET_RTC_SEC());

	tsc1 = READ_TSC();
	second2 = GET_RTC_SEC();

	while(SEC_MINUS(second2, second1) < 1) {
		second2 = GET_RTC_SEC();
		tsc2 = READ_TSC();
	}

	//Return value = number-of-clock-cycles-per-second / number-of-ticks-per-second
	return (tsc2 - tsc1) / (SEC_MINUS(second2, second1) * 1000 / SYS_TICK);
}
