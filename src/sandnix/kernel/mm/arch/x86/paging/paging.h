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

#include "../../../../../../common/common.h"
#include "../../../../rtl/rtl.h"
#include "../page_table.h"

#define	KERNEL_MEM_BASE			0xC0000000
#define	KERNEL_MEM_SIZE			(1024*1024*1024)

#define	MAX_PAGEBLOCK_SIZE		4096

#ifndef	_ASM

extern	void*	mm_mgr_page_addr;
extern	void*	mm_init_pt_addr;
extern	void*	kernel_address_offset;
extern	u32		init_page_num;

void	paging_init_arch(plist_t p_kernel_free_page_list,
                         plist_t p_kernel_using_page_list,
                         void* heap);
#endif	//!	_ASM
