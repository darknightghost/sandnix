/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

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

#include "../../../../../common/common.h"
#include "../../../hal/debug/debug.h"
#include "../../pm/pm.h"

//Magic
#define	HEAP_MEMBLOCK_MAGIC		0xFFF23333

//Heap attributes
#define HEAP_MULITHREAD         0x00000001
#define HEAP_PREALLOC           0x00000002


//Heap page block attribute
#define	HEAP_PAGE_BLOCK_FIX		0x00000001

#define	HEAP_MEM_BLOCK_BLACK	0x00000000
#define	HEAP_MEM_BLOCK_RED		0x00000001

struct	_heap_t;

typedef	struct _heap_pg_blck_t {
    struct _heap_pg_blck_t* p_prev;
    struct _heap_pg_blck_t* p_next;
    u32						attr;
    size_t					size;
    u64						ref;
    struct _heap_t*			p_heap;
} heap_pg_blck_t, *pheap_pg_blck_t;

typedef struct _heap_mem_blck_t {
    u32							magic;
    pheap_pg_blck_t				p_pg_block;
    struct	_heap_mem_blck_t*	p_parent;
    struct	_heap_mem_blck_t*	p_lchild;
    struct	_heap_mem_blck_t*	p_rchild;
    u32							color;
    u32							size;
    struct _heap_t*				p_heap;
} heap_mem_blck_t, *pheap_mem_blck_t;

typedef struct _heap_t {
    u32					type;
    pheap_pg_blck_t		p_pg_block_list;
    pheap_mem_blck_t	p_used_block_tree;
    pheap_mem_blck_t	p_empty_block_tree;
    spnlck_t			lock;
} heap_t, *pheap_t;

//heap
//Create heap
pheap_t core_mm_heap_create(
    u32 attribute,
    size_t scale);

//Create heap on buffer
pheap_t core_mm_heap_create_on_buf(
    u32 attribute,
    size_t scale,
    void* init_buf,
    size_t init_buf_size);

//Allocate memory from heap
void* core_mm_heap_alloc(
    size_t size,
    pheap_t heap);

//Release memory from heap
void core_mm_heap_free(
    size_t size,
    pheap_t heap);

//Destroy the heap
void core_mm_heap_destroy(
    size_t size,
    pheap_t heap);

//Check if heap is corrupted
void core_mm_heap_chk(pheap_t heap);

#define HEAP_CHECK(heap) { \
        do { \
            if(hal_is_on_debug()) { \
                core_mm_heap_chk((heap)); \
            } \
        } while(0); \
    }

