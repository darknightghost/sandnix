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

#define	SYSTEM_CONTROL_PORTA                         0x92

//CMOS RAM index register port
#define	CMOS_RAM_INDEX_PORT                          0x70
#define	NMI_EN_PORT                                  0x70


//PIC-8259 ports
#define	MASTER_ICW1_PORT                             0x20
#define	MASTER_ICW2_PORT                             0x21
#define	MASTER_ICW3_PORT                             0x21
#define	MASTER_ICW4_PORT                             0x21
#define	MASTER_OCW1_PORT                             0x21
#define	MASTER_OCW2_PORT                             0x20
#define	MASTER_OCW3_PORT                             0x20

#define	SLAVE_ICW1_PORT                              0xa0
#define	SLAVE_ICW2_PORT                              0xa1
#define	SLAVE_ICW3_PORT                              0xa1
#define	SLAVE_ICW4_PORT                              0xa1
#define	SLAVE_OCW1_PORT                              0xa1
#define	SLAVE_OCW2_PORT                              0xa0
#define	SLAVE_OCW3_PORT                              0xa0

#define	MASTER_MASK_PORT                             0x21
#define	MASTER_EOI_PORT                              0x20
#define	MASTER_IMR_PORT                              0x21
#define	MASTER_IRR_PORT                              0x20
#define	MASTER_ISR_PORT                              0x20

#define	SLAVE_MASK_PORT                              0xa1
#define	SLAVE_EOI_PORT                               0xa0
#define	SLAVE_IMR_PORT                               0xa1
#define	SLAVE_IRR_PORT                               0xa0
#define	SLAVE_ISR_PORT                               0xa0


//PIT - 8253 ports
#define	PIT_COUNTER0_PORT                            0x40
#define	PIT_COUNTER1_PORT                            0x41
#define	PIT_COUNTER2_PORT                            0x42
#define	PIT_CONTROL_PORT                             0x43


//PCI configuration port
#define	PCI_CONFIG_ADDRESS                           0xcf8
#define	PCI_CONFIG_DATA                              0xcfc

//Reset control register
#define	RESET_CONTROL_REGISTER                       0xcf9
#define	FAST_A20_INIT_REGISTER                       0x92

//APM controll port
#define	APM_CONTROL_PORT_REGISTER                    0xb2
#define	APM_CNT                                      0xb2
#define	APM_STS                                      0xb3


//Keyboard controller & encoder ports
#define	I8408_DATA_PORT                              0x60
#define	I8408_COMMAND_PORT                           0x60
#define	I8402_STATUS_PORT                            0x64
#define	I8402_COMMAND_PORT                           0x64
