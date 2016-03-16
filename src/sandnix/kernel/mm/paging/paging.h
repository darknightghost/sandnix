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
#include "page_obj.h"

#ifdef	x86
	#include "../arch/x86/paging/paging.h"
#endif	//	X86

#define	MEM_USER
#define	MEM_COMMIT
#define	MEM_RESERVE
#define	MEM_UNCOMMIT
#define	MEM_RELEASE
#define	MEM_DMA

typedef	struct	_page_block {
	size_t		index;
	size_t		num;
	ppage_obj_t	p_page_obj;
} page_block_t, *ppage_block_t;

typedef	struct	_page_block_list {
	size_t	each_block_num;
	list_t	block_list;
} page_block_list_t, *ppage_block_list_t;

typedef	struct	_page_globl_dir {
	id_t				begin;
	size_t				num;
	ppage_proc_dir_t	p_proc_dirs;
} page_globl_dir_t, *ppage_globl_dir_t;

typedef	struct	_page_proc_dir {
	void*		physical_address;
} page_proc_dir_t, *ppage_proc_dir_t;

void		paging_init();
void*		mm_page_alloc(void* base, size_t num, u32 options);
void*		mm_page_obj_map(void* base, ppage_obj_t p_obj);
void		mm_page_free(void* base);
ppage_obj_t	mm_get_page_obj(void* base);
