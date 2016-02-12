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

#pragma once

#define IRQ0_HI_INDEX	0x10
#define IRQ0_LO_INDEX	0x11
#define IRQ0_INDEX		0x10
#define IRQ1_INDEX		0x12
#define IRQ2_INDEX		0x14
#define IRQ3_INDEX		0x16
#define IRQ4_INDEX		0x18
#define IRQ5_INDEX		0x1A
#define IRQ6_INDEX		0x1C
#define IRQ7_INDEX		0x1E
#define IRQ8_INDEX		0x20
#define IRQ9_INDEX		0x22
#define IRQ10_INDEX		0x24
#define IRQ11_INDEX		0x26
#define IRQ12_INDEX		0x28
#define IRQ13_INDEX		0x2A
#define IRQ14_INDEX		0x2C
#define IRQ15_INDEX		0x2E
#define IRQ16_INDEX		0x30
#define IRQ17_INDEX		0x32
#define IRQ18_INDEX		0x34
#define IRQ19_INDEX		0x36
#define IRQ20_INDEX		0x38
#define IRQ21_INDEX		0x3A
#define IRQ22_INDEX		0x3C
#define IRQ23_INDEX		0x3E

extern	u32		apic_base_addr;
extern	u32		io_apic_base_addr;
extern	u32*	p_io_apic_index;
extern	u32*	p_io_apic_data;
extern	u32*	p_io_apic_eoi;

void	apic_init();

