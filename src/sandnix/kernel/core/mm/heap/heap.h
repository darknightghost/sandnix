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

#include "./heap_defs.h"

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
    void* p_mem,
    pheap_t heap);

//Destroy the heap
void core_mm_heap_destroy(
    size_t size,
    pheap_t heap);

//Check if heap is corrupted
void core_mm_heap_chk(pheap_t heap);

#define HEAP_CHECK(heap) { \
        do { \
            if(hal_debug_is_on_dbg()) { \
                core_mm_heap_chk((heap)); \
            } \
        } while(0); \
    }


