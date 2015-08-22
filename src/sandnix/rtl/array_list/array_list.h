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

#include "../rtl.h"

typedef	struct	_array_list_node {
	size_t	scale;
	size_t	remains;
	void**	p_datas;
} array_list_node_t, *parray_list_node_t, *array_list_t;

k_status		rtl_array_list_init(array_list_t* p_array,	//Which array
                                    void* heap,				//Which heap
                                    size_t size);			//How many items
void*			rtl_array_list_get(array_list_t array, u32 index, void* heap);
k_status		rtl_array_list_set(array_list_t array, u32 index, void* value, void* heap);
void			rtl_array_list_release(array_list_t array, u32 index, void* heap);

#endif	//!	ARRAY_LIST_H_INCLUDE
