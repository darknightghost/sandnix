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

#ifndef	HEAP_H_INCLUDE
#define	HEAP_H_INCLUDE

#include "../mm.h"
#include "../../pm/pm.h"

#define	HEAP_INTACT_MAGIC			(*((u32*)"HEAP"))
#define	KERNEL_DEFAULT_HEAP_SIZE	(4096*4)

#ifdef	X86
	#pragma pack(4)
#endif	//X86

typedef	struct _mem_block_head {
	u32							magic;
	struct _mem_block_head*		p_prev_empty_block;
	struct _mem_block_head*		p_next_empty_block;
	u8*							start_addr;
	int							allocated_flag;
	size_t						size;
} mem_block_head_t, *pmem_block_head_t;

typedef	struct _heap_head {
	struct _heap_head*			p_prev;
	struct _heap_head*			p_next;
	pmem_block_head_t			p_first_empty_block;
	size_t						scale;
	u32							attr;
	spinlock_t					lock;
} heap_head_t, *pheap_head_t;

#pragma pack()

#endif	//!	HEAP_H_INCLUDE
