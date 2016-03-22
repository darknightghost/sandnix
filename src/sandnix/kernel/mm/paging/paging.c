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

#include "../../../../common/common.h"
#include "../../rtl/rtl.h"
#include "../../pm/pm.h"
#include "../../debug/debug.h"
#include "../../exceptions/exceptions.h"
#include "paging.h"
#include "../heap/heap.h"

#define	PAGING_HEAP_SIZE	4096
#define	PG_GLOBL_DIR_NUM	(1 << (MAX_PROC_NUM_INDEX / 2))

static	list_t				kernel_free_blocks;
static	list_t				kernel_using_blocks;
static	page_globl_dir_t	page_dir[PG_GLOBL_DIR_NUM];
static	void*				paging_heap;
static	u8					paging_heap_buf[PAGING_HEAP_SIZE];

void paging_init()
{
	id_t current_id;

	dbg_kprint("Initialize paging...\n");
	kernel_using_blocks = NULL;
	//paging_heap = mm_hp_create_on_buf(paging_heap_buf, PAGING_HEAP_SIZE,
	//                                  HEAP_EXTENDABLE | HEAP_MULTITHREAD
	//                                  | HEAP_PREALLOC);
	paging_heap = mm_hp_create_on_buf(paging_heap_buf, PAGING_HEAP_SIZE,
	                                  HEAP_MULTITHREAD
	                                  | HEAP_PREALLOC);
	rtl_memset(page_dir, 0, sizeof(page_dir));
	paging_init_arch(&kernel_free_blocks, &kernel_using_blocks, paging_heap);
	UNREFERRED_PARAMETER(current_id);
}

void*		mm_page_alloc(void* base, size_t num, u32 options);
void*		mm_page_obj_map(void* base, ppage_obj_t p_obj);
void		mm_page_free(void* base);
ppage_obj_t	mm_get_page_obj(void* base);
