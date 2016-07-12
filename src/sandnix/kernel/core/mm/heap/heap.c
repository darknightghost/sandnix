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

#include "heap.h"

static	bool	is_default_heap_ready = false;
static	u8		default_heap_pg_block[4096 * 2];
static	heap_t	default_heap;

#define INIT_HEAPA(p_heap, heap_type) { \
        do { \
            (p_heap)->type = (heap_type); \
            (p_heap)->p_pg_block_list = NULL; \
            (p_heap)->p_used_block_tree = NULL; \
            (p_heap)->p_empty_block_tree = NULL; \
            core_pm_spnlck_init(&((p_heap)->lock)); \
        }while(0); \
    }

#define INIT_PG_BLOCK(p_block, blk_attr, blk_size, p_heap1) { \
        do { \
            ((pheap_pg_blck_t)(p_block))->p_prev = NULL; \
            ((pheap_pg_blck_t)(p_block))->p_next = NULL; \
            ((pheap_pg_blck_t)(p_block))->index = 0; \
            ((pheap_pg_blck_t)(p_block))->attr = (blk_attr); \
            ((pheap_pg_blck_t)(p_block))->size = (blk_size); \
            ((pheap_pg_blck_t)(p_block))->ref = 0; \
            ((pheap_pg_blck_t)(p_block))->p_heap = (p_heap1); \
        }while(0); \
    }

#define	INIT_MEM_BLOCK(p_block, blk_size, p_pg_block1, p_heap1) { \
        do { \
            ((pheap_mem_blck_t)(p_block))->magic = HEAP_MEMBLOCK_MAGIC; \
            ((pheap_mem_blck_t)(p_block))->p_pg_block = (p_pg_block1); \
            ((pheap_mem_blck_t)(p_block))->p_parent = NULL; \
            ((pheap_mem_blck_t)(p_block))->p_lchild = NULL; \
            ((pheap_mem_blck_t)(p_block))->p_rchild = NULL; \
            ((pheap_mem_blck_t)(p_block))->color = HEAP_MEM_BLOCK_BLACK; \
            ((pheap_mem_blck_t)(p_block))->size = (blk_size); \
            ((pheap_mem_blck_t)(p_block))->p_heap = (p_heap1); \
        } while(0); \
    }

static	void				init_default_heap();
static	pheap_mem_blck_t	get_free_mem_block(pheap_t p_heap, size_t size);

pheap_t core_mm_heap_create(
    u32 attribute,
    size_t scale);

pheap_t core_mm_heap_create_on_buf(
    u32 attribute,
    size_t scale,
    void* init_buf,
    size_t init_buf_size);

void* core_mm_heap_alloc(size_t size, pheap_t heap)
{
    pheap_t p_heap;
    pheap_mem_blck_t p_mem_block;

    //Get heap
    if(heap == NULL) {
        p_heap = &default_heap;

        if(!is_default_heap_ready) {
            init_default_heap();
        }

    } else {
        p_heap = heap;
    }

    if(p_heap->type & HEAP_MULITHREAD) {
        core_pm_spnlck_lock(p_heap->lock);
    }

    //Get mem block
    p_mem_block = get_free_mem_block(p_heap, size);

    if(p_mem_block == NULL) {
        //TODO:Allocate more pages
    }

    if(p_mem_block->size > size + sizeof(heap_mem_blck_t) * 2) {
        //Split the block
    }

    if(p_heap->type & HEAP_PREALLOC) {
        //Pre-allocate page
    }

    if(p_heap->type & HEAP_MULITHREAD) {
        core_pm_spnlck_unlock(p_heap->lock);
    }

    return (address_t)p_mem_block + sizeof(heap_mem_blck_t);
}

void core_mm_heap_free(
    size_t size,
    pheap_t heap);

void core_mm_heap_destroy(
    size_t size,
    pheap_t heap);

void core_mm_heap_chk(pheap_t heap);


void init_default_heap()
{
    pheap_mem_blck_t p_mem_block;

    //Initialize heap
    INIT_HEAPA(&default_heap, HEAP_MULITHREAD);

    //Initialize page block
    INIT_PG_BLOCK(default_heap_pg_block, HEAP_PAGE_BLOCK_FIX,
                  sizeof(default_heap_pg_block), &default_heap);
    default_heap.p_pg_block_list = (pheap_pg_blck_t)default_heap_pg_block;

    //Initialize first mem block
    p_mem_block = (pheap_mem_blck_t)((address_t)default_heap_pg_block
                                     + sizeof(heap_mem_blck_t));

    if(((address_t)p_mem_block) % 8 > 0) {
        p_mem_block = (pheap_mem_blck_t)(8 - (address_t)p_mem_block % 8
                                         + (address_t)p_mem_block);
    }

    INIT_MEM_BLOCK(p_mem_block, sizeof(default_heap_pg_block)
                   - ((address_t)p_mem_block - (address_t)default_heap_pg_block),
                   (pheap_pg_blck_t)default_heap_pg_block , &default_heap);

    default_heap.p_empty_block_tree = p_mem_block;
    return;
}
