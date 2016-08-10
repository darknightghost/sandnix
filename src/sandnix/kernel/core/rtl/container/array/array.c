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

#include "array.h"
#include "../../../mm/mm.h"
#include "../../../../hal/exception/exception.h"
#include "../../string/string.h"

kstatus_t core_rtl_array_init(parray_t p_array, u32 num, pheap_t heap)
{
    u32 count = 0;
    u32 n = num;

    while(n != 0) {
        count++;
        n = n >> 1;
    }

    if(num - (1 << count) > 0) {
        count++;
    }

    p_array->size = (1 << count);
    p_array->scale = (1 << (count / 2));
    p_array->heap = heap;

    p_array->p_p_blks = core_mm_heap_alloc(
                            p_array->size / p_array->scale * sizeof(parray_blk_t),
                            heap);

    if(p_array->p_p_blks == NULL) {
        return ENOMEM;
    }

    return ESUCCESS;
}

void* core_rtl_array_get(parray_t p_array, u32 index)
{
    if(index >= p_array->size) {
        return NULL;
    }

    //Get block
    parray_blk_t* p_p_blk = p_array->p_p_blks + index / p_array->scale;

    if(*p_p_blk == NULL) {
        return NULL;
    }

    //Get item
    return (*p_p_blk)->p_p_items + index % p_array->scale;
}

void* core_rtl_array_set(parray_t p_array, u32 index, void* value)
{
    //Get block
    parray_blk_t* p_p_blk = p_array->p_p_blks + index / p_array->scale;

    if(value == NULL && *p_p_blk == NULL) {
        return NULL;

    } else {
        //Allocate new block
        *p_p_blk = core_mm_heap_alloc(sizeof(array_blk_t), p_array->heap);

        if(*p_p_blk == NULL) {
            return NULL;
        }

        (*p_p_blk)->ref = 0;
        (*p_p_blk)->p_p_items = core_mm_heap_alloc(sizeof(void*)*p_array->scale,
                                p_array->heap);

        if((*p_p_blk)->p_p_items == NULL) {
            core_mm_heap_free((*p_p_blk)->p_p_items, p_array->heap);
            *p_p_blk = NULL;
            return NULL;
        }

        core_rtl_memset((*p_p_blk)->p_p_items, 0, sizeof(void*)*p_array->scale);
    }

    parray_blk_t p_blk = (*p_p_blk);

    void** p_p_item = p_blk->p_p_items + index % p_array->scale;

    if(value == NULL) {
        if(*p_p_item == NULL) {
            return NULL;

        } else {
            *p_p_item = NULL;
            (p_blk->ref)--;

            if(p_blk->ref == 0) {
                core_mm_heap_free(p_blk->p_p_items, p_array->heap);
                core_mm_heap_free(p_blk, p_array->heap);
                *p_p_blk = NULL;
            }

            return NULL;
        }

    } else {
        if(*p_p_item == NULL) {
            (p_blk->ref)++;
        }

        *p_p_item = value;
        return value;
    }
}

bool core_rtl_array_used(parray_t p_array, u32 index)
{
    //Get block
    parray_blk_t* p_p_blk = p_array->p_p_blks + index / p_array->scale;

    if(*p_p_blk == NULL) {
        return false;

    }

    parray_blk_t p_blk = (*p_p_blk);
    void** p_p_item = p_blk->p_p_items + index % p_array->scale;

    if(*p_p_item == NULL) {
        return false;
    }

    return true;
}

u32 core_rtl_array_size(parray_t p_array)
{
    return p_array->size;
}

bool core_rtl_array_get_used_index(parray_t p_array, u32 begin, u32* ret)
{
    for(u32 i = begin / p_array->scale;
        i < p_array->size / p_array->scale;
        i++) {
        parray_blk_t* p_p_blk = p_array->p_p_blks + i / p_array->scale;

        if(*p_p_blk != NULL) {
            for(u32 j = 0; j < p_array->scale; j++) {
                void** p_p_item = (*p_p_blk)->p_p_items + j;

                if(*p_p_item != NULL) {
                    *ret = i * p_array->scale + j;
                    return true;
                }
            }
        }

    }

    return false;
}

bool core_rtl_array_get_free_index(parray_t p_array, u32* p_ret)
{
    for(u32 i = 0;
        i < p_array->size / p_array->scale;
        i++) {
        parray_blk_t* p_p_blk = p_array->p_p_blks + i / p_array->scale;

        if(*p_p_blk == NULL) {
            return i * p_array->scale;

        } else {
            if((*p_p_blk)->ref < p_array->scale) {
                for(u32 j = 0; j < p_array->scale; j++) {
                    void** p_p_item = (*p_p_blk)->p_p_items + j;

                    if(*p_p_item == NULL) {
                        *p_ret = i * p_array->scale + j;
                        return true;
                    }
                }
            }
        }

    }

    return false;
}

u32 core_rtl_array_get_free_index_num(parray_t p_array)
{
    u32 ret = 0;

    for(u32 i = 0; i < p_array->size / p_array->scale; i++) {
        parray_blk_t* p_p_blk = p_array->p_p_blks + i / p_array->scale;

        if(*p_p_blk == NULL) {
            ret += p_array->scale;

        } else {
            ret += p_array->scale - (*p_p_blk)->ref;
        }
    }

    return ret;
}

void core_rtl_array_destroy(parray_t p_array, item_destroyer_t destroier,
                            void* arg)
{
    for(u32 i = 0;
        i < p_array->size / p_array->scale;
        i++) {
        parray_blk_t* p_p_blk = p_array->p_p_blks + i / p_array->scale;

        if(*p_p_blk != NULL) {
            for(u32 j = 0; j < p_array->scale; j++) {
                void** p_p_item = (*p_p_blk)->p_p_items + j;

                if(*p_p_item != NULL) {
                    if(destroier != NULL) {
                        destroier(*p_p_item, arg);
                    }
                }
            }

            core_mm_heap_free((*p_p_blk)->p_p_items, p_array->heap);
            core_mm_heap_free(*p_p_blk, p_array->heap);
        }
    }

    core_mm_heap_free(p_array->p_p_blks, p_array->heap);
    return;
}
