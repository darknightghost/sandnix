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
#include "../frame.h"
#include "../../../mm/mm.h"

typedef struct	_rbtree_node {
    struct _rbtree_node*	p_parent;
    struct _rbtree_node*	p_lchild;
    struct _rbtree_node*	p_rchild;
    u32						color;
    void*					p_key;
    void*					p_value;
} rbtree_node_t, *prbtree_node_t, *map_t, **pmap_t;

//Initialize
void core_rtl_map_init(
    pmap_t p_map,
    item_compare_t compare_func,
    pheap_t heap);

//Set key value, If value == NULL, the key will be removed.
void* core_rtl_map_set(
    pmap_t p_map,
    void* p_key,
    void* p_value);

//Get key value
void* core_rtl_map_get(
    pmap_t p_map,
    void* p_key);

//Get next key
void* core_rtl_map_next(
    pmap_t p_map,
    void* p_key);

//Destroy the map
void core_rtl_map_destroy(
    pmap_t p_map,
    item_destroyer_t destroier,
    void* arg);
