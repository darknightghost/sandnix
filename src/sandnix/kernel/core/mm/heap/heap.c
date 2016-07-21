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
#include "../../../hal/exception/exception.h"

static	bool							is_default_heap_ready = false;
static __attribute__((aligned(8)))	u8	default_heap_pg_block[4096 * 2];
static	heap_t							default_heap;

#define	DEFAULT_HEAP_SCALE		(sizeof(default_heap_pg_block) / 4096 * 4096)

#define INIT_HEAP(p_heap, hp_scale, heap_type) { \
        do { \
            (p_heap)->type = (heap_type); \
            (p_heap)->scale = (hp_scale); \
            (p_heap)->p_pg_block_list = NULL; \
            (p_heap)->p_used_block_tree = NULL; \
            (p_heap)->p_empty_block_tree = NULL; \
            core_pm_spnlck_init(&((p_heap)->lock)); \
            core_pm_spnlck_init(&((p_heap)->prealloc_lock)); \
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

#define	INIT_MEMBLOCK(p_block, blk_size, p_pg_block1, p_heap1) { \
        do { \
            ((pheap_mem_blck_t)(p_block))->magic = HEAP_MEMBLOCK_MAGIC; \
            ((pheap_mem_blck_t)(p_block))->allocated = false; \
            ((pheap_mem_blck_t)(p_block))->p_prev = NULL; \
            ((pheap_mem_blck_t)(p_block))->p_next = NULL; \
            ((pheap_mem_blck_t)(p_block))->p_pg_block = (p_pg_block1); \
            ((pheap_mem_blck_t)(p_block))->p_parent = NULL; \
            ((pheap_mem_blck_t)(p_block))->p_lchild = NULL; \
            ((pheap_mem_blck_t)(p_block))->p_rchild = NULL; \
            ((pheap_mem_blck_t)(p_block))->color = HEAP_MEMBLOCK_BLACK; \
            ((pheap_mem_blck_t)(p_block))->size = (blk_size); \
            ((pheap_mem_blck_t)(p_block))->p_heap = (p_heap1); \
        } while(0); \
    }

#define	IS_BLACK(p_node)	((p_node) == NULL \
                             || (p_node)->color == HEAP_MEMBLOCK_BLACK)
#define	IS_RED(p_node)		((p_node) != NULL \
                             && (p_node)->color == HEAP_MEMBLOCK_RED)

static	void				init_default_heap();
static	pheap_mem_blck_t	get_free_mem_block(pheap_t p_heap, size_t size);
static	void				insert_node(php_mem_blck_tree p_tree,
                                        pheap_mem_blck_t p_node);
static	void				remove_node(php_mem_blck_tree p_tree,
                                        pheap_mem_blck_t p_node);
static	pheap_mem_blck_t	l_rotate(php_mem_blck_tree p_tree,
                                     pheap_mem_blck_t p_node);
static	pheap_mem_blck_t	r_rotate(php_mem_blck_tree p_tree,
                                     pheap_mem_blck_t p_node);
static	void				check_tree(php_mem_blck_tree p_tree);
static	void				check_node(pheap_mem_blck_t p_node, long* p_num,
                                       long count);
static	int					compare_node(pheap_mem_blck_t p1, pheap_mem_blck_t p2);

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
    pheap_mem_blck_t p_new_mem_block;

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
        core_pm_spnlck_lock(&(p_heap->lock));
    }

    size = (size % 8 ? (size / 8 + 1) * 8 : size);

    //Get mem block
    p_mem_block = get_free_mem_block(p_heap, size);

    if(p_mem_block == NULL) {
        //TODO:Allocate more pages
        NOT_SUPPORT;
    }

    remove_node(&(p_heap->p_empty_block_tree),
                p_mem_block);

    if(p_mem_block->size > size + HEAP_MEM_BLCK_SZ * 2 + 8) {
        //Split the block
        p_new_mem_block = (pheap_mem_blck_t)((address_t)p_mem_block + size
                                             + HEAP_MEM_BLCK_SZ);
        INIT_MEMBLOCK(p_new_mem_block,
                      p_mem_block->size - size - HEAP_MEM_BLCK_SZ,
                      p_mem_block->p_pg_block,
                      p_mem_block->p_heap);
        p_mem_block->size = size + HEAP_MEM_BLCK_SZ;
        insert_node(&(p_heap->p_empty_block_tree),
                    p_new_mem_block);
        p_new_mem_block->p_next = p_mem_block->p_next;
        p_new_mem_block->p_prev = p_mem_block;
        p_mem_block->p_next = p_new_mem_block;

        if(p_new_mem_block->p_next != NULL) {
            p_new_mem_block->p_next->p_prev = p_new_mem_block;
        }

    }

    insert_node(&(p_heap->p_used_block_tree),
                p_mem_block);
    p_mem_block->allocated = true;
    (p_mem_block->p_pg_block->ref)++;

    if(p_heap->type & HEAP_MULITHREAD) {
        core_pm_spnlck_unlock(&(p_heap->lock));
    }

    if(p_heap->type & HEAP_PREALLOC) {
        //TODO:Pre-allocate page
        NOT_SUPPORT;
    }

    HEAP_CHECK(p_heap);

    return (void*)((address_t)p_mem_block + HEAP_MEM_BLCK_SZ);
}

void core_mm_heap_free(
    void* p_mem,
    pheap_t heap)
{
    pheap_t p_heap;
    pheap_mem_blck_t p_mem_block;
    pheap_mem_blck_t p_prev_mem_block;
    pheap_mem_blck_t p_next_mem_block;

    //Get heap
    if(heap == NULL) {
        p_heap = &default_heap;

    } else {
        p_heap = heap;
    }

    if(p_heap->type & HEAP_MULITHREAD) {
        core_pm_spnlck_lock(&(p_heap->lock));
    }

    //Release memory block
    p_mem_block = (pheap_mem_blck_t)((address_t)p_mem - HEAP_MEM_BLCK_SZ);
    remove_node(&(p_heap->p_used_block_tree),
                p_mem_block);
    p_mem_block->allocated = false;
    (p_mem_block->p_pg_block->ref)--;

    //Release page
    if(p_mem_block->p_pg_block->ref == 0
       && (p_mem_block->p_pg_block->attr & HEAP_PAGEBLOCK_FIX)) {
        //TODO:Release page
        NOT_SUPPORT;
    }

    //Join memory blocks
    //Prev block
    p_prev_mem_block = p_mem_block->p_prev;

    if(p_prev_mem_block != NULL
       && !p_prev_mem_block->allocated) {
        remove_node(&(p_heap->p_empty_block_tree), p_prev_mem_block);
        p_prev_mem_block->size += p_mem_block->size;
        p_prev_mem_block->p_next = p_mem_block->p_next;

        if(p_mem_block->p_next != NULL) {

            p_mem_block->p_next->p_prev = p_prev_mem_block;
        }

        p_mem_block = p_prev_mem_block;
    }

    //Next block
    p_next_mem_block = p_mem_block->p_next;

    if(p_next_mem_block != NULL
       && !p_next_mem_block->allocated) {
        remove_node(&(p_heap->p_empty_block_tree), p_next_mem_block);
        p_mem_block->size += p_next_mem_block->size;
        p_mem_block->p_next = p_next_mem_block->p_next;

        if(p_mem_block->p_next != NULL) {
            p_mem_block->p_next->p_prev = p_mem_block;
        }
    }

    insert_node(&(p_heap->p_empty_block_tree), p_mem_block);

    if(p_heap->type & HEAP_MULITHREAD) {
        core_pm_spnlck_unlock(&(p_heap->lock));
    }

    HEAP_CHECK(p_heap);

    return;
}

void core_mm_heap_destroy(
    size_t size,
    pheap_t heap);

void core_mm_heap_chk(pheap_t heap)
{
    pheap_t p_heap;
    pheap_pg_blck_t p_pg_blk;
    pheap_mem_blck_t p_mem_blk;

    if(heap == NULL) {
        p_heap = &default_heap;

    } else {
        p_heap = heap;
    }

    if(p_heap->type & HEAP_MULITHREAD) {
        core_pm_spnlck_unlock(&(p_heap->lock));
    }

    //Check memory blocks
    for(p_pg_blk = p_heap->p_pg_block_list;
        p_pg_blk != NULL;
        p_pg_blk = p_pg_blk->p_next) {
        for(p_mem_blk = (pheap_mem_blck_t)((address_t)p_pg_blk
                                           + HEAP_PG_BLCK_SZ);
            p_mem_blk != NULL;
            p_mem_blk = p_mem_blk->p_next) {
            if(p_mem_blk->magic != HEAP_MEMBLOCK_MAGIC) {
                hal_exception_panic(EHPCORRUPTION,
                                    "Corruption has been detected in heap %p.",
                                    p_heap);
            }
        }
    }

    //Check trees
    check_tree(&(p_heap->p_empty_block_tree));

    check_tree(&(p_heap->p_used_block_tree));

    if(p_heap->type & HEAP_MULITHREAD) {
        core_pm_spnlck_unlock(&(p_heap->lock));
    }

    return;
}


void init_default_heap()
{
    pheap_mem_blck_t p_mem_block;

    //Initialize heap
    INIT_HEAP(&default_heap, DEFAULT_HEAP_SCALE, HEAP_MULITHREAD);

    //Initialize page block
    INIT_PG_BLOCK(default_heap_pg_block, HEAP_PAGEBLOCK_FIX,
                  DEFAULT_HEAP_SCALE, &default_heap);
    default_heap.p_pg_block_list = (pheap_pg_blck_t)default_heap_pg_block;

    //Initialize first memory block
    p_mem_block = (pheap_mem_blck_t)((address_t)default_heap_pg_block
                                     + HEAP_PG_BLCK_SZ);

    INIT_MEMBLOCK(p_mem_block, DEFAULT_HEAP_SCALE
                  - HEAP_PG_BLCK_SZ,
                  (pheap_pg_blck_t)default_heap_pg_block , &default_heap);

    default_heap.p_empty_block_tree = p_mem_block;
    return;
}

pheap_mem_blck_t get_free_mem_block(pheap_t p_heap, size_t size)
{
    pheap_mem_blck_t p_block;

    //No free block remains
    if(p_heap->p_empty_block_tree == NULL) {
        return NULL;
    }

    //Search for node
    size += HEAP_MEM_BLCK_SZ;
    p_block = p_heap->p_empty_block_tree;

    while(true) {
        if(p_block->size == size) {
            return p_block;
        }

        if(p_block->size > size) {
            if(p_block->p_lchild == NULL) {
                while(p_block->size < size
                      && p_block != NULL) {
                    p_block = p_block->p_parent;
                }

                return p_block;

            } else {
                p_block = p_block->p_lchild;
            }

        } else {
            if(p_block->p_rchild == NULL) {
                while(p_block->size < size
                      && p_block != NULL) {
                    p_block = p_block->p_parent;
                }

                return p_block;

            } else {
                p_block = p_block->p_rchild;
            }
        }
    }
}

void insert_node(php_mem_blck_tree p_tree,
                 pheap_mem_blck_t p_node)
{
    pheap_mem_blck_t p_insert_node;
    pheap_mem_blck_t p_z;
    pheap_mem_blck_t p_y;
    int cmp_result;

    p_node->p_parent = NULL;
    p_node->p_lchild = NULL;
    p_node->p_rchild = NULL;

    if(*p_tree == NULL) {
        //The tree is empty
        *p_tree = p_node;
        p_node->color = HEAP_MEMBLOCK_BLACK;

    } else {
        //Find where to insert the node
        p_insert_node = *p_tree;

        while(true) {
            cmp_result = compare_node(p_node, p_insert_node);

            if(cmp_result <= 0) {
                if(p_insert_node->p_lchild == NULL) {
                    p_insert_node->p_lchild = p_node;
                    p_node->p_parent = p_insert_node;
                    break;

                } else {
                    p_insert_node = p_insert_node->p_lchild;
                }

            } else {
                if(p_insert_node->p_rchild == NULL) {
                    p_insert_node->p_rchild = p_node;
                    p_node->p_parent = p_insert_node;
                    break;

                } else {
                    p_insert_node = p_insert_node->p_rchild;
                }
            }
        }

        p_node->color = HEAP_MEMBLOCK_RED;

        //Fix tree
        p_z = p_node;

        while(p_z->p_parent != NULL
              && p_z->p_parent->color == HEAP_MEMBLOCK_RED) {
            if(p_z->p_parent == p_z->p_parent->p_parent->p_lchild) {
                p_y = p_z->p_parent->p_parent->p_rchild;

                if(p_y != NULL && p_y->color == HEAP_MEMBLOCK_RED) {
                    p_z->p_parent->color = HEAP_MEMBLOCK_BLACK;
                    p_y->color = HEAP_MEMBLOCK_BLACK;
                    p_z->p_parent->p_parent->color = HEAP_MEMBLOCK_RED;
                    p_z = p_z->p_parent->p_parent;

                } else if(p_z == p_z->p_parent->p_rchild) {
                    p_z = p_z->p_parent;
                    l_rotate(p_tree, p_z);
                }

                if(p_z->p_parent != NULL
                   && p_z->p_parent->p_parent != NULL) {
                    p_z->p_parent->color = HEAP_MEMBLOCK_BLACK;
                    p_z->p_parent->p_parent->color = HEAP_MEMBLOCK_RED;
                    r_rotate(p_tree, p_z->p_parent->p_parent);
                }

            } else {
                p_y = p_z->p_parent->p_parent->p_lchild;

                if(p_y == NULL && p_y->color == HEAP_MEMBLOCK_RED) {
                    p_z->p_parent->color = HEAP_MEMBLOCK_BLACK;
                    p_y->color = HEAP_MEMBLOCK_BLACK;
                    p_z->p_parent->p_parent->color = HEAP_MEMBLOCK_RED;
                    p_z = p_z->p_parent->p_parent;

                } else if(p_z == p_z->p_parent->p_lchild) {
                    p_z = p_z->p_parent;
                    r_rotate(p_tree, p_z);
                }

                if(p_z->p_parent != NULL
                   && p_z->p_parent->p_parent != NULL) {
                    p_z->p_parent->color = HEAP_MEMBLOCK_BLACK;
                    p_z->p_parent->p_parent->color = HEAP_MEMBLOCK_RED;
                    l_rotate(p_tree, p_z->p_parent->p_parent);
                }
            }
        }

        (*p_tree)->color = HEAP_MEMBLOCK_BLACK;
    }

    return;
}

void remove_node(php_mem_blck_tree p_tree,
                 pheap_mem_blck_t p_node)
{
    //TODO:
    UNREFERRED_PARAMETER(p_tree);
    UNREFERRED_PARAMETER(p_node);
    return;
}

pheap_mem_blck_t l_rotate(php_mem_blck_tree p_tree,
                          pheap_mem_blck_t p_node)
{
    pheap_mem_blck_t p_parent;
    pheap_mem_blck_t p_x;
    pheap_mem_blck_t p_y;

    p_x = p_node;
    p_parent = p_x->p_parent;

    if(p_node->p_rchild == NULL) {
        hal_exception_panic(EHPCORRUPTION,
                            "Corruption has been detected in memory block %p.",
                            p_node);
    }

    p_y = p_node->p_rchild;
    p_x->p_rchild = p_y->p_lchild;

    if(p_x->p_rchild != NULL) {
        p_x->p_rchild->p_parent = p_x;
    }

    p_y->p_lchild = p_x;
    p_x->p_parent = p_y;

    if(p_parent == NULL) {
        *p_tree = p_y;
        p_y->p_parent = NULL;

    } else {
        if(p_parent->p_lchild == p_x) {
            p_parent->p_lchild = p_y;

        } else {
            p_parent->p_rchild = p_y;
        }

        p_y->p_parent = p_parent;
    }

    return p_y;
}

pheap_mem_blck_t r_rotate(php_mem_blck_tree p_tree,
                          pheap_mem_blck_t p_node)
{
    pheap_mem_blck_t p_parent;
    pheap_mem_blck_t p_x;
    pheap_mem_blck_t p_y;

    p_x = p_node;
    p_parent = p_x->p_parent;

    if(p_node->p_lchild == NULL) {
        hal_exception_panic(EHPCORRUPTION,
                            "Corruption has been detected in memory block %p.",
                            p_node);
    }

    p_y = p_node->p_lchild;
    p_x->p_lchild = p_y->p_rchild;

    if(p_x->p_lchild != NULL) {
        p_x->p_lchild->p_parent = p_x;
    }

    p_y->p_rchild = p_x;
    p_x->p_parent = p_y;

    if(p_parent == NULL) {
        *p_tree = p_y;
        p_y->p_parent = NULL;

    } else {
        if(p_parent->p_lchild == p_x) {
            p_parent->p_lchild = p_y;

        } else {
            p_parent->p_rchild = p_y;
        }

        p_y->p_parent = p_parent;
    }

    return p_y;
}

void check_tree(php_mem_blck_tree p_tree)
{
    long num;

    if(*p_tree == NULL) {
        return;

    } else if((*p_tree)->color == HEAP_MEMBLOCK_RED) {
        hal_exception_panic(EHPCORRUPTION,
                            "Corruption has been detected in memory block %p.",
                            p_tree);
    }

    num = -1;
    check_node(*p_tree, &num, 0);

    return;
}

void check_node(pheap_mem_blck_t p_node, long * p_num, long count)
{
#define	CHECK_COUNT(p_num, count) { \
        do { \
            if(*(p_num) == -1) { \
                *(p_num) = (count); \
            } else if(*(p_num) != (count)) { \
                \
            } \
        } while(0); \
    }

    if(p_node->magic != HEAP_MEMBLOCK_MAGIC) {
        hal_exception_panic(EHPCORRUPTION,
                            "Corruption has been detected in memory block %p.",
                            p_node);
    }

    if(p_node->color == HEAP_MEMBLOCK_RED) {
        if(IS_RED(p_node->p_parent)
           || IS_RED(p_node->p_lchild)
           || IS_RED(p_node->p_rchild)) {
            hal_exception_panic(EHPCORRUPTION,
                                "Corruption has been detected in memory block %p.",
                                p_node);
        }

        if(p_node->p_lchild == NULL) {
            CHECK_COUNT(p_num, count);

        } else {
            check_node(p_node->p_lchild, p_num, count);
        }

        if(p_node->p_rchild == NULL) {
            CHECK_COUNT(p_num, count);

        } else {
            check_node(p_node->p_rchild, p_num, count);
        }

    } else if(p_node->color == HEAP_MEMBLOCK_BLACK) {
        if(p_node->p_lchild == NULL) {
            CHECK_COUNT(p_num, count + 1);

        } else {
            check_node(p_node->p_lchild, p_num, count + 1);
        }

        if(p_node->p_rchild == NULL) {
            CHECK_COUNT(p_num, count + 1);

        } else {
            check_node(p_node->p_rchild, p_num, count + 1);
        }

    } else {
        hal_exception_panic(EHPCORRUPTION,
                            "Corruption has been detected in memory block %p.",
                            p_node);
    }

    return;
}

int compare_node(pheap_mem_blck_t p1, pheap_mem_blck_t p2)
{
    if(p1->size > p2->size) {
        return 1;

    } else if(p1->size < p2->size) {
        return -1;

    } else {
        if(p1->p_pg_block->index > p2->p_pg_block->index) {
            return 1;

        } else if(p1->p_pg_block->index < p2->p_pg_block->index) {
            return -1;

        } else {
            return 0;
        }
    }
}
