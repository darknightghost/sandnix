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

#include "../../../../../../common/common.h"

#ifndef CORE_RTL_EXPORT
    #include "../../../mm/mm.h"
#endif

typedef struct _heap_t	heap_t, *pheap_t;

typedef	struct	_array_blk {
    size_t		ref;
    void**		p_p_items;
} array_blk_t, *parray_blk_t;

#include "../container.h"

typedef	struct	_array {
    size_t			size;
    size_t			scale;
    pheap_t			heap;
    parray_blk_t*	p_p_blks;
} array_t, *parray_t;

//Initialize.
kstatus_t core_rtl_array_init(
    parray_t p_array,		//Array
    u32 num,				//Max num
    pheap_t heap);			//heap

//Get item.
void* core_rtl_array_get(
    parray_t p_array,		//Array
    u32 index);				//Index

//Set item. NULL will remove the item.
void* core_rtl_array_set(
    parray_t p_array,		//Array
    u32 index,				//Index
    void* value);			//Value

//Check if the index is used.
bool core_rtl_array_used(
    parray_t p_array,		//Array
    u32 index);				//Index

//Get the number of item.
u32 core_rtl_array_size(
    parray_t p_arra);		//Array

//Get a used index
bool core_rtl_array_get_used_index(
    parray_t p_array,		//Array
    u32 begin,				//Begining index
    u32* ret);				//Next index

//Get a free index
bool core_rtl_array_get_free_index(
    parray_t p_array,		//Array
    u32* p_ret);			//Index

//Get the number of free indexes
u32 core_rtl_array_get_free_index_num(
    parray_t p_array);		//Array

//Destroy the array
void core_rtl_array_destroy(
    parray_t p_array,			//Array
    item_destroyer_t destroier,	//Destroy callback
    void* arg);					//Argument of callback
