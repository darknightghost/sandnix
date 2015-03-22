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

#ifndef	INTERRUPT_H_INCLUDE
#define	INTERRUPT_H_INCLUDE

#include "../types.h"
#include "../segment.h"

#pragma	pack(1)

typedef struct idt_reg {
	u16		limit;
	u32		base;
} idt_reg, *pidt_reg;

typedef	struct	_idt {
	u16		offset1;			//0-15 bits of offset
	u16		selector;			//CS selector
	struct {
		u16		reserved: 5;	//Zero
		u16		zero: 3;		//Zero
		u16		type: 4;		//Interrupt or trap
		u16		s: 1;			//Zero
		u16		dpl: 2;			//Dpl
		u16		p: 1;			//Segment present flag
	} attr;
	u16		offset2;			//16-32 bits of offset
} idt, *pidt;
#pragma	pack()

#define		TYPE_TRAP			0x0F
#define		TYPE_INTERRUPT		0x0E

#define		INTERRUPT_MAX_NUM		256
#define		IDT_BASE_ADDR			0x00000100
#define		GET_IDT(num)			((pidt)(IDT_BASE_ADDR+(num)*sizeof(idt)))
#define		SET_IDT(num,func,cs_selector,d_type,d_s,d_dpl,s_p) {\
		GET_IDT((num))->offset1=(u32)GET_REAL_ADDR((func))&0x0000FFFF;\
		GET_IDT((num))->offset2=(((u32)GET_REAL_ADDR((func))&0xFFFF0000)>>16);\
		GET_IDT((num))->selector=(cs_selector);\
		GET_IDT((num))->attr.reserved=0;\
		GET_IDT((num))->attr.zero=0;\
		GET_IDT((num))->attr.type=d_type;\
		GET_IDT((num))->attr.s=d_s;\
		GET_IDT((num))->attr.dpl=d_dpl;\
		GET_IDT((num))->attr.p=s_p;\
	}

void		setup_interrupt();

#endif	//! INTERRUPT_H_INCLUDE
