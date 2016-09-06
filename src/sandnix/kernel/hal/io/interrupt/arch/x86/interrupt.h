/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

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
#include "../../../../../../../common/common.h"
#include "../../interrupt.h"
#include "../../../../cpu/cpu.h"
#include "apic.h"
#include "tss.h"

#define	INT_BP			0x03

#define	REQUIRE_EOI_BEGIN	0x20
#define	INT_IPI			0x20

#define	INT_TICK		0x21

#define	IRQ_BASE		0x22
#define	IRQ(n)			(IRQ_BASE + (n))

#define	IRQ_MAX			IRQ(23)
#define	INT_CLOCK		IRQ(2)
#define	REQUIRE_EOI_END	IRQ_MAX

u32		hal_io_apic_read32(address_t off);
void	hal_io_apic_write32(address_t off, u32 data);
