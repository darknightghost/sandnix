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

#define	HEAP_INTACT_MAGIC			(*((u32*)"HEAP"))
#define	KERNEL_DEFAULT_HEAP_SIZE	(4096*2)

#define	HEAP_EXTENDABLE				0x01
#define	HEAP_SWAPPABLE				0x02
#define	HEAP_MULTITHREAD			0x04
#define	HEAP_DESTROY				0x08

#pragma pack(4)
typedef	struct _mem_block_head {
	u32							magic;
	struct _mem_block_head*		p_prev_empty_block;
	struct _mem_block_head*		p_next_empty_block;
	u8*							start_addr;
	int							allocated_flag;
	size_t						size;
} mem_block_head, *pmem_block_head;

typedef	struct _heap_head {
	struct _heap_head*			p_prev;
	struct _heap_head*			p_next;
	pmem_block_head				p_first_empty_block;
	size_t						scale;
	u32							attr;
} heap_head, *pheap_head;

#pragma pack()

void		mm_heap_create();
void		mm_heap_destroy();
void*		mm_heap_alloc(size_t size, void* heap_addr);
bool		mm_heap_chk(void* heap_addr);
void		mm_heap_free(void* addr, void* heap_addr);

#endif	//!	HEAP_H_INCLUDE
