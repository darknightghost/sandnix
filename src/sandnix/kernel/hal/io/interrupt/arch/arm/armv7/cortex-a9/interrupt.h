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
#include "../../../../../../../../../common/common.h"
#include "../../../../interrupt.h"
#include "../../../../../../cpu/cpu.h"
#include "samsung_exynos4412/gic.h"
#include "ivt.h"

#define	INT_UNDEF			0x00
#define	INT_PREFETCH_ABT	0x01
#define	INT_DATA_ABT		0x02
#define INT_BP				0x03
#define	INT_GATE			0x04

#define	REQUIRE_EOI_BEGIN	0x20
#define	INT_IPI			0x20

#define	INT_TICK		0x21

#define	IRQ_BASE		0x22
#define	IRQ0			(IRQ_BASE)
#define	IRQ1			(IRQ_BASE + 1)
#define	IRQ2			(IRQ_BASE + 2)
#define	IRQ3			(IRQ_BASE + 3)
#define	IRQ4			(IRQ_BASE + 4)
#define	IRQ5			(IRQ_BASE + 5)
#define	IRQ6			(IRQ_BASE + 6)
#define	IRQ7			(IRQ_BASE + 7)
#define	IRQ8			(IRQ_BASE + 8)
#define	IRQ9			(IRQ_BASE + 9)
#define	IRQ10			(IRQ_BASE + 10)
#define	IRQ11			(IRQ_BASE + 11)
#define	IRQ12			(IRQ_BASE + 12)
#define	IRQ13			(IRQ_BASE + 13)
#define	IRQ14			(IRQ_BASE + 14)
#define	IRQ15			(IRQ_BASE + 15)
#define	IRQ16			(IRQ_BASE + 16)
#define	IRQ17			(IRQ_BASE + 17)
#define	IRQ18			(IRQ_BASE + 18)
#define	IRQ19			(IRQ_BASE + 19)
#define	IRQ20			(IRQ_BASE + 20)
#define	IRQ21			(IRQ_BASE + 21)
#define	IRQ22			(IRQ_BASE + 22)
#define	IRQ23			(IRQ_BASE + 23)

#define	IRQ_MAX			IRQ23
#define	IRQ_CLOCK		IRQ2
#define	REQUIRE_EOI_END	IRQ_MAX
