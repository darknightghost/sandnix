/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "../../../../common/common.h"
#include "../../rtl/rtl.h"

#ifdef	X86
	#include "../arch/x86/phymem/phymem.h"
#endif	//!	X86

#define	PHY_MEM_ALLOCATABLE		0
#define	PHY_MEM_ALLOCATED		1
#define	PHY_MEM_RESERVED		2
#define	PHY_MEM_SYSTEM			3
#define	PHY_MEM_BAD				4

typedef	struct	_phymem_tbl_entry {
	void*	base;
	size_t	size;
	u32		status;
} phymem_tbl_entry_t, *pphymem_tbl_entry_t;

extern	list_t		phymem_list;
extern	list_t		phymem_allocatable_list;

void	phymem_init();