/*
	Copyright 2015,∞µ“π”ƒ¡È <darknightghost.cn@gmail.com>

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

#ifndef	MEMORY_H_INCLUDE
#define	MEMORY_H_INCLUDE

#include "../types.h"

#define	HEAP_BASE		(void*)0x00040000
#define	HEAP_SIZE		0x50000

typedef struct _mem_block_head {
	struct _mem_block_head*		p_prev;
	struct _mem_block_head*		p_next;
	void*						start_addr;
	int							allocated_flag;
	size_t						size;
} mem_block_head_t, *pmem_block_head_t;

//Initialize heap
void		init_heap();
//Allocate memory
void*		malloc(size_t size);
//Free memory
void		free(void* addr);

#endif	//! MEMORY_H_INCLUDE
