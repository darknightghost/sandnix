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

#include "array_list.h"
#include "../../mm/mm.h"
#include "../../pm/pm.h"
#include "../../exceptions/exceptions.h"

k_status rtl_array_list_init(array_list_t* p_array,
                             void* heap,
                             size_t size)
{
	if(size < 4) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	//Compute num
	p_array->size = size;
	p_array->num = (u32)rtl_sqrt(size);

	while(size % (p_array->num) != 0) {
		(p_array->num)--;
	}

	p_array->p_nodes = mm_hp_alloc(sizeof(void*) * (p_array->num), heap);

	if(p_array->p_nodes == NULL) {
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	rtl_memset(p_array->p_nodes, 0, sizeof(void*) * (p_array->num));
	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

void* rtl_array_list_get(parray_list_t p_array, u32 index)
{
	parray_list_node_t p_node;

	if(index >= p_array->size) {
		pm_set_errno(EINVAL);
		return NULL;
	}

	p_node = *(p_array->p_nodes + index / ((p_array->size) / (p_array->num)));

	if(p_node == NULL) {
		pm_set_errno(ESUCCESS);
		return NULL;
	}

	pm_set_errno(ESUCCESS);
	return (p_node->p_datas + index % (p_node->scale));
}

k_status rtl_array_list_set(parray_list_t p_array, u32 index, void* p_item, void* heap)
{
	parray_list_node_t* p_p_node;
	parray_list_node_t p_node;
	void** p_p_item;

	if(index >= p_array->size) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	p_p_node = p_array->p_nodes + index / ((p_array->size) / (p_array->num));

	if(*p_p_node == NULL) {
		*p_p_node = mm_hp_alloc(sizeof(array_list_node_t), heap);

		if(*p_p_node == NULL) {
			pm_set_errno(EFAULT);
			return EFAULT;
		}

		p_node = *p_p_node;
		p_node->scale = (p_array->size) / (p_array->num);
		p_node->remains = p_node->scale;
		p_node->p_datas = mm_hp_alloc((p_node->scale) * sizeof(void*),
		                              heap);

		if(p_node->p_datas == NULL) {
			mm_hp_free(p_node->p_datas, heap);
			*p_p_node = NULL;
			pm_set_errno(EFAULT);
			return EFAULT;
		}

		rtl_memset(p_node->p_datas,
		           0,
		           (p_node->scale) * sizeof(void*));
	}

	p_node = *p_p_node;
	p_p_item = p_node->p_datas + index % p_node->scale;

	if(*p_p_item == NULL) {
		(p_node->remains)--;
	}

	*p_p_item = p_item;

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

void rtl_array_list_release(parray_list_t p_array, u32 index, void* heap)
{
	parray_list_node_t* p_p_node;
	parray_list_node_t p_node;
	void** p_p_item;

	if(index >= p_array->size) {
		pm_set_errno(ESUCCESS);
		return;
	}

	p_p_node = p_array->p_nodes + index / ((p_array->size) / (p_array->num));

	if(*p_p_node == NULL) {
		pm_set_errno(ESUCCESS);
		return;
	}

	p_node = *p_p_node;
	p_p_item = p_node->p_datas + index % p_node->scale;

	if(*p_p_item != NULL) {
		(p_node->remains)++;
	}

	if(p_node->scale == p_node->remains) {
		mm_hp_free(p_node, heap);
		*p_p_node = NULL;

	} else {
		*p_p_item = NULL;
	}

	pm_set_errno(ESUCCESS);
	return;
}

u32 rtl_array_list_get_free_index(parray_list_t p_array)
{
	u32 index;
	parray_list_node_t* p_p_node;
	parray_list_node_t p_node;
	void** p_p_item;

	p_p_node = p_array->p_nodes;

	for(index = 0; index < p_array->size;) {
		p_p_node = p_array->p_nodes + index / ((p_array->size) / (p_array->num));

		if(*p_p_node == NULL) {
			pm_set_errno(ESUCCESS);
			return index;
		}

		p_node = *p_p_node;

		if(p_node->remains != 0) {
			for(p_p_item = p_node->p_datas;
			    *p_p_item != NULL;
			    p_p_item++, index++);

			pm_set_errno(ESUCCESS);
			return index;

		} else {
			index += p_node->scale;
		}
	}

	pm_set_errno(ENOMEM);
	return 0;
}

