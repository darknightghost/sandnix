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

#ifndef	PAGING_H_INCLUDE
#define	PAGING_H_INCLUDE

#include "../../../../../common/arch/x86/types.h"
#include "../../../mm.h"

#define	PAGE_TABLE_VADDR
#define	REFRESH_TLB	{\
		__asm__ __volatile__(\
		                     "movl	%%cr4,%%eax\n\t"\
		                     "btcl	$7,%%eax\n\t"\
		                     "movl	%%eax,%%cr4\n\t"\
		                     "pushl	%%eax\n\t"\
		                     "movl	%%cr3,%%eax\n\t"\
		                     "movl	%%eax,%%cr3\n\t"\
		                     "popl	%%eax\n\t"\
		                     "btsl	$7,%%eax\n\t"\
		                     "movl	%%eax,%%cr4\n\t"\
		                     ::);\
	}

void		init_paging();
#endif	//!	PAGING_H_INCLUDE
