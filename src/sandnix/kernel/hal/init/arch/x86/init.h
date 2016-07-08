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
#include "../../../../../../common/common.h"

#define	DA_32		0x4000		//32 bit segment
#define	DA_DPL0		0x00		//DPL = 0
#define	DA_DPL1		0x20		//DPL = 1
#define	DA_DPL2		0x40		//DPL = 2
#define	DA_DPL3		0x60		//DPL = 3

//Data segment
#define	DA_DR		0x90		//Read-only
#define	DA_DRW		0x92		//Readable & writeable
#define	DA_DRWA		0x93		//Readable writeable & accessed

//Code segment
#define	DA_C		0x98		//Excute-only
#define	DA_CR		0x9A		//Excuteable & readable
#define	DA_CCO		0x9C		//Excute-only conforming code segment
#define	DA_CCOR		0x9E		//Excuteable & readable conforming code segment

#define	DA_LDT		0x82		//LDT descriptor
#define	DA_TASKGATE	0x85		//Task gate
#define	DA_386TSS	0x89		//TSS
#define	DA_386CGATE	0x8C		//Call gate
#define	DA_386IGATE	0x8E		//Interrupt gate
#define	DA_386TGATE	0x8F		//Trap gate

//Segment selectors
#define	DESCRIPTOR_SIZE			8
#define	SELECTOR_K_DATA			(1 * DESCRIPTOR_SIZE)
#define	SELECTOR_K_CODE			(2 * DESCRIPTOR_SIZE)
#define	SELECTOR_U_DATA			(3 * DESCRIPTOR_SIZE | 3)
#define	SELECTOR_U_CODE			(4 * DESCRIPTOR_SIZE | 3)
#define	SELECTOR_BASIC_VIDEO	(5 * DESCRIPTOR_SIZE)
#define	SELECTOR_TSS			(6 * DESCRIPTOR_SIZE)
#define	BASIC_VIDEO_BASE_ADDR	0xC00B8000

#ifdef	_ASM
    .macro		SEGMENT_DESCRIPTOR base, limit, property
    //Limit1
    .word(\limit) & 0xFFFF
    // Base1
    .word(\base) & 0xFFFF
    //Base2
    .byte((\base) >> 16) & 0xFF
    //Property1+Limit2+property2,G=1
    .word(((\limit) >> 8) & 0x0F00) | ((\property) & 0xF0FF) | 0x8000
    //Base3
    .byte((\base) >> 24) & 0xFF
    .endm
#endif

#ifndef	_ASM
#pragma pack(push)
#pragma pack(1)

typedef	struct	_gdt_t {
    u16		limit;
    u16		base1;
    u8		base2;
    u16		attr;
    u8		base3;
} gdt_t, *pgdt_t;

extern	pgdt_t		gdt;
#pragma pack(pop)
#endif
