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

void	idt_init();
