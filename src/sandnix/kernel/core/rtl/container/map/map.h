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

#include "../../../mm/mm_defs.h"

typedef struct _heap_t	heap_t, *pheap_t;

#include "../container_defs.h"

#include "./map_defs.h"

//Initialize
void core_rtl_map_init(
    pmap_t p_map,
    item_compare_t compare_func,
    pheap_t heap);

//Set key value, If value == NULL, the key will be removed.
//On sucess, if the operation is an insert operation, the return value is p_value,
//otherwise, the return value is the previous value.
//On failed, the function returns NULL
void* core_rtl_map_set(
    pmap_t p_map,
    void* p_key,
    void* p_value);

//Get key value
void* core_rtl_map_get(
    pmap_t p_map,
    void* p_key);

//Get prev key
void* core_rtl_map_prev(
    pmap_t p_map,
    void* p_key);

//Get next key
void* core_rtl_map_next(
    pmap_t p_map,
    void* p_key);

//Search key and value
//int	(*map_search_func_t)(void* p_condition, void* p_key, void* p_value,void* p_arg);
typedef	int	(*map_search_func_t)(void*, void*, void*, void*);
void* core_rtl_map_search(
    pmap_t p_map,
    void* p_condition,
    map_search_func_t search_func,
    void* p_arg);

//Destroy the map
void core_rtl_map_destroy(
    pmap_t p_map,
    item_destroyer_t key_destroier,
    item_destroyer_t value_destroier,
    void* arg);
