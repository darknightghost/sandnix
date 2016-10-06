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

#include "../../../hal/debug/debug_defs.h"
#include "../../pm/pm_defs.h"

//Magic
#define	HEAP_MEMBLOCK_MAGIC		0xFFF23333

//Heap attributes
#define HEAP_MULITHREAD         0x00000001
#define HEAP_PREALLOC           0x00000002


//Heap page block attribute
#define	HEAP_PAGEBLOCK_FIX		0x00000001

#define	HEAP_MEMBLOCK_BLACK		0x00000000
#define	HEAP_MEMBLOCK_RED		0x00000001

struct	_heap_t;

typedef	struct _heap_pg_blck_t {
    struct _heap_pg_blck_t* p_prev;
    struct _heap_pg_blck_t* p_next;
    u32						index;
    u32						attr;
    size_t					size;
    u64						ref;
    struct _heap_t*			p_heap;
} heap_pg_blck_t, *pheap_pg_blck_t;

#define	HEAP_PG_BLCK_SZ		(sizeof(heap_pg_blck_t) % 8 \
                             ? (sizeof(heap_pg_blck_t) / 8 + 1) * 8 \
                             : sizeof(heap_pg_blck_t))

typedef struct _heap_mem_blck_t {
    u32							magic;
    bool						allocated;
    struct	_heap_mem_blck_t*	p_prev;
    struct	_heap_mem_blck_t*	p_next;
    struct	_heap_mem_blck_t*	p_parent;
    struct	_heap_mem_blck_t*	p_lchild;
    struct	_heap_mem_blck_t*	p_rchild;
    pheap_pg_blck_t				p_pg_block;
    u32							color;
    u32							size;
    struct _heap_t*				p_heap;
} heap_mem_blck_t, *pheap_mem_blck_t, *hp_mem_blck_tree, **php_mem_blck_tree;

#define	HEAP_MEM_BLCK_SZ	(sizeof(heap_mem_blck_t) % 8 \
                             ? (sizeof(heap_mem_blck_t) / 8 + 1) * 8 \
                             : sizeof(heap_mem_blck_t))


typedef struct _heap_t {
    u32					type;
    pheap_pg_blck_t		p_pg_block_list;
    hp_mem_blck_tree	p_used_block_tree;
    hp_mem_blck_tree	p_empty_block_tree;
    size_t				scale;
    spnlck_t			lock;
    spnlck_t			prealloc_lock;
} heap_t, *pheap_t;
