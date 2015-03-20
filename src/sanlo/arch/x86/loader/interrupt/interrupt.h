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


#pragma	pack(1)
typedef	struct	_idt{
	u16		offset1;			//0-15 bits of offset
	u16		selector;			//Selector
	struct{
		u16		reserved:5;		//Zero
		u16		zero:3;			//Zero
		u16		type:4;			//Interrupt or trap
		u16		s:1;			//Zero
		u16		dpl:2;			//Dpl
		u16		p:1;			//Segment present flag
	}attr;
	u16		offset2;			//16-32 bits of offset
}idt,*pidt;
#pragma	pack()

void		interrupt_init();

#endif	//! INTERRUPT_H_INCLUDE