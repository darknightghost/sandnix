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

typedef struct _heap_t		heap_t, *pheap_t;

typedef struct	_rbtree_node {
    struct _rbtree_node*	p_parent;
    struct _rbtree_node*	p_lchild;
    struct _rbtree_node*	p_rchild;
    u32						color;
    void*					p_key;
    void*					p_value;
} rbtree_node_t, *prbtree_node_t, *rbtree_t, **prbtree_t;

typedef struct _heap_t	heap_t, *pheap_t;

#include "../container_defs.h"

typedef struct _map {
    rbtree_t		p_tree;
    item_compare_t	compare_func;
    pheap_t			p_heap;
} map_t, *pmap_t;

#define	RBTREE_NODE_BLACK		0
#define	RBTREE_NODE_RED			1

//Search key and value
//int	(*map_search_func_t)(void* p_condition, void* p_key, void* p_value,void* p_arg);
typedef	int	(*map_search_func_t)(void*, void*, void*, void*);

#include "../../../mm/mm_defs.h"
