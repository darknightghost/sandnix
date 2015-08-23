/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#ifndef	ARRAY_LIST_H_INCLUDE
#define	ARRAY_LIST_H_INCLUDE

#include "../../../common/common.h"
#include "../list/list.h"

typedef	struct	_array_list_node {
	size_t	scale;				//Maxium iterms in the node
	size_t	remains;			//How many iterms can be addded
	void**	p_datas;			//Iterm table
} array_list_node_t, *parray_list_node_t;

typedef	struct {
	u32					num;		//Number of nodes
	u32					size;		//Maxium iterms
	parray_list_node_t*	p_nodes;	//Node table
} array_list_t, *parray_list_t;

k_status		rtl_array_list_init(parray_list_t p_array,	//Which array
                                    size_t size,			//How many items
                                    void* heap);			//Which heap
void*			rtl_array_list_get(parray_list_t p_array, u32 index);
k_status		rtl_array_list_set(parray_list_t p_array, u32 index, void* p_item, void* heap);
void			rtl_array_list_release(parray_list_t p_array, u32 index, void* heap);
u32				rtl_array_list_get_free_index(parray_list_t p_array);
void			rtl_array_list_destroy(parray_list_t p_array,
                                       item_destroyer_callback callback,
                                       void* heap);

#endif	//!	ARRAY_LIST_H_INCLUDE
