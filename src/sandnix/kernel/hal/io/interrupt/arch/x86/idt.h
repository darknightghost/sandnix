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

#include "../../interrupt.h"

#ifndef	HAL_IO_EXPORT
    #include "../../../../init/init.h"
#endif

typedef struct {
    u16		limit;
    u32		base;
} __attribute__((packed)) idt_reg_t, *pidt_reg_t;

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
    } __attribute__((packed)) attr;
    u16		offset2;			//16-32 bits of offset
} __attribute__((packed)) idt_t, *pidt_t;

#define		TYPE_TRAP			0x0F
#define		TYPE_INTERRUPT		0x0E

#define		INTERRUPT_MAX_NUM	256
#define		SET_IDT(base, num, func, cs_selector, d_type, d_s, d_dpl, s_p) { \
        (base)[(num)].offset1=(u32)(func)&0x0000FFFF; \
        (base)[(num)].offset2=(((u32)(func)&0xFFFF0000)>>16); \
        (base)[(num)].selector=(cs_selector); \
        (base)[(num)].attr.reserved=0; \
        (base)[(num)].attr.zero=0; \
        (base)[(num)].attr.type=(d_type); \
        (base)[(num)].attr.s=(d_s); \
        (base)[(num)].attr.dpl=(d_dpl); \
        (base)[(num)].attr.p=(s_p); \
    }

#define	SET_NORMAL_IDT(base,num) SET_IDT((base), num, int_##num, SELECTOR_K_CODE, TYPE_INTERRUPT, 0, 0, 1)

void	idt_init();
