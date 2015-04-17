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

#ifndef	INT_H_INCLUDE
#define	INT_H_INCLUDE

#include "../../io.h"

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
		u16		dpl: 2;			//DPL
		u16		p: 1;			//Segment present flag
	} attr;
	u16		offset2;			//16-32 bits of offset
} idt, *pidt;
#pragma	pack()

#define		TYPE_TRAP			0x0F
#define		TYPE_INTERRUPT		0x0E

#define		INTERRUPT_MAX_NUM		256
#define		GET_IDT(base,num)			((pidt)((base)+(num)*sizeof(idt)))
#define		SET_IDT(base,num,func,cs_selector,d_type,d_s,d_dpl,s_p) {\
		GET_IDT((base),(num))->offset1=(u32)GET_REAL_ADDR((func))&0x0000FFFF;\
		GET_IDT((base),(num))->offset2=(((u32)GET_REAL_ADDR((func))&0xFFFF0000)>>16);\
		GET_IDT((base),(num))->selector=(cs_selector);\
		GET_IDT((base),(num))->attr.reserved=0;\
		GET_IDT((base),(num))->attr.zero=0;\
		GET_IDT((base),(num))->attr.type=d_type;\
		GET_IDT((base),(num))->attr.s=d_s;\
		GET_IDT((base),(num))->attr.dpl=d_dpl;\
		GET_IDT((base),(num))->attr.p=s_p;\
	}

#define	INT_DE			0x00
#define	INT_DB			0x01
#define	INT_NMI			0x02
#define	INT_BP			0x03
#define	INT_OF			0x04
#define	INT_BR			0x05
#define	INT_UD			0x06
#define	INT_NM			0x07
#define	INT_DF			0x08
#define	INT_FPU			0x09
#define	INT_TS			0x0A
#define	INT_NP			0x0B
#define	INT_SS			0x0C
#define	INT_GP			0x0D
#define	INT_PF			0x0E
#define	INT_RESERVED	0x0F
#define	INT_MF			0x10
#define	INT_AC			0x11
#define	INT_MC			0x12
#define	INT_XF			0x13

#define	IRQ0			0x20
#define	IRQ1			0x21
#define	IRQ3			0x23
#define	IRQ4			0x24
#define	IRQ5			0x25
#define	IRQ6			0x26
#define	IRQ7			0x27
#define	IRQ8			0x28
#define	IRQ9			0x29
#define	IRQ10			0x2A
#define	IRQ11			0x2B
#define	IRQ12			0x2C
#define	IRQ13			0x2D
#define	IRQ14			0x2E
#define	IRQ15			0x2F

void		init_idt();
#endif	//!	INT_H_INCLUDE
