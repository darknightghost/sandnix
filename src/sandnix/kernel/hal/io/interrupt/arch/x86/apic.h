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

#define	LOCAL_APIC_ID_REG		0x00000020
#define	LOCAL_APIC_VERSION_REG	0x00000030
#define	LOCAL_APIC_TPR_REG		0x00000080
#define	LOCAL_APIC_APR_REG		0x00000090
#define	LOCAL_APIC_PPR_REG		0x000000A0
#define	LOCAL_APIC_EOI_REG		0x000000B0
#define	LOCAL_APIC_RRD_REG		0x000000C0
#define	LOCAL_APIC_LDR_REG		0x000000D0
#define	LOCAL_APIC_DFR_REG		0x000000E0
#define	LOCAL_APIC_SVR_REG		0x000000F0

#define	LOCAL_APIC_ISR0_REG		0x00000100
#define	LOCAL_APIC_ISR1_REG		0x00000110
#define	LOCAL_APIC_ISR2_REG		0x00000120
#define	LOCAL_APIC_ISR3_REG		0x00000130
#define	LOCAL_APIC_ISR4_REG		0x00000140
#define	LOCAL_APIC_ISR5_REG		0x00000150
#define	LOCAL_APIC_ISR6_REG		0x00000160
#define	LOCAL_APIC_ISR7_REG		0x00000170

#define	LOCAL_APIC_TMR0_REG		0x00000180
#define	LOCAL_APIC_TMR1_REG		0x00000190
#define	LOCAL_APIC_TMR2_REG		0x000001A0
#define	LOCAL_APIC_TMR3_REG		0x000001B0
#define	LOCAL_APIC_TMR4_REG		0x000001C0
#define	LOCAL_APIC_TMR5_REG		0x000001D0
#define	LOCAL_APIC_TMR6_REG		0x000001E0
#define	LOCAL_APIC_TMR7_REG		0x000001F0

#define	LOCAL_APIC_IRR0_REG		0x00000200
#define	LOCAL_APIC_IRR1_REG		0x00000210
#define	LOCAL_APIC_IRR2_REG		0x00000220
#define	LOCAL_APIC_IRR3_REG		0x00000230
#define	LOCAL_APIC_IRR4_REG		0x00000240
#define	LOCAL_APIC_IRR5_REG		0x00000250
#define	LOCAL_APIC_IRR6_REG		0x00000260
#define	LOCAL_APIC_IRR7_REG		0x00000270

#define	LOCAL_APIC_ESR_REG		0x00000280

#define	LOCAL_APIC_LVT_CMCI_REG					0x000002F0
#define	LOCAL_APIC_ICR0_REG						0x00000300
#define	LOCAL_APIC_ICR1_REG						0x00000310
#define	LOCAL_APIC_LVT_TIMER_REG				0x00000320
#define	LOCAL_APIC_LVT_THERMAL_SENSOR_REG		0x00000330
#define	LOCAL_APIC_LVT_PERFORM_MON_COUNT_REG	0x00000340
#define	LOCAL_APIC_LVT_LINT0_REG				0x00000350
#define	LOCAL_APIC_LVT_LINT1_REG				0x00000360
#define	LOCAL_APIC_LVT_ERR_REG					0x00000370

#define	LOCAL_APIC_TIMER_INITIAL_COUNT_REG		0x00000380
#define	LOCAL_APIC_TIMER_CURRENT_COUNT_REG		0x00000390
#define	LOCAL_APIC_TIMER_DIVIDE_CONF_REG		0x000003E0

void	apic_init();

//Read local APIC register
u32		hal_io_apic_read32(address_t off);

//Write local APIC register
void	hal_io_apic_write32(address_t off, u32 data);
