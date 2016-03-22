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
#ifndef	_ASM
	#include "page_obj.h"
#endif	//!	_ASM

#ifdef	X86
	#include "../arch/x86/paging/paging.h"
#endif	//	X86

#define	PAGE_USER		0x00000001
#define	PAGE_COMMIT		0x00000002
#define	PAGE_RESERVE	0x00000004
#define	PAGE_UNCOMMIT	0x00000008
#define	PAGE_RELEASE	0x00000010
#define	PAGE_DMA		0x00000020

#ifndef	_ASM
typedef	struct	_page_block {
	size_t		index;				//The index of first page
	size_t		num;				//Number of pages the block has
	ppage_obj_t	p_page_obj;			//Null if not allocated
} page_block_t, *ppage_block_t;

typedef	struct	_page_block_list {
	size_t	each_block_num;			//Number of pages each block has
	list_t	block_list;				//Blocks
} page_block_list_t, *ppage_block_list_t;

typedef	struct	_page_proc_dir {
	void*		physical_address;	//Physical address of page table of the process
} page_proc_dir_t, *ppage_proc_dir_t;

typedef	struct	_page_globl_dir {
	id_t				begin;		//The begining page table id
	size_t				num;		//How many page table does it has
	ppage_proc_dir_t	p_proc_dirs;//Pointer to page directories
} page_globl_dir_t, *ppage_globl_dir_t;

void		paging_init();
void*		mm_page_alloc(void* base, size_t num, u32 options);
void*		mm_page_obj_map(void* base, ppage_obj_t p_obj);
void		mm_page_free(void* base);
ppage_obj_t	mm_get_page_obj(void* base);

#endif	//!	_ASM
