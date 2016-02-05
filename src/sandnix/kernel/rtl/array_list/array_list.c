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

k_status rtl_array_list_init(parray_list_t p_array,
                             size_t size,
                             void* heap)

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

	//Allocate memory for node table
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

	//Get the node
	p_p_node = p_array->p_nodes + index / ((p_array->size) / (p_array->num));

	if(*p_p_node == NULL) {
		//Allocate memory for new node
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

	//Get the node
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
		//Release the node
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
		//Search for empty item
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

u32 rtl_array_list_get_next_index(parray_list_t p_array, u32 index)
{
	parray_list_node_t p_node;
	size_t scale;

	scale = p_array->size / p_array->num;

	while(index < p_array->size) {
		p_node = *(p_array->p_nodes + scale);

		if(p_node == NULL) {
			//Jump to next node
			index = (index / scale + 1) * scale;
			continue;
		}

		if(*(p_node->p_datas + index % scale) != NULL) {
			pm_set_errno(ESUCCESS);
			return index;
		}

		index++;
	}

	pm_set_errno(EOVERFLOW);
	return index;
}

void rtl_array_list_destroy(parray_list_t p_array,
                            item_destroyer_callback callback,
                            void* p_arg,
                            void* heap)
{
	parray_list_node_t* p_p_node;
	u32 i, j;
	void** p_p_item;

	for(i = 0, p_p_node = p_array->p_nodes;
	    i < p_array->num;
	    i++, p_p_node++) {
		if(*p_p_node != NULL) {
			if(callback != NULL) {
				for(j = 0, p_p_item = (*p_p_node)->p_datas;
				    j < (*p_p_node)->scale;
				    j++, p_p_item++) {
					if(*p_p_item != NULL) {
						callback(*p_p_item, p_arg);
					}
				}
			}

			mm_hp_free((*p_p_node)->p_datas, heap);
			mm_hp_free(*p_p_node, heap);
		}
	}

	mm_hp_free(p_array->p_nodes, heap);

	pm_set_errno(ESUCCESS);
	return;
}

size_t rtl_array_list_item_num(parray_list_t p_array)
{
	size_t count;
	u32 i;
	parray_list_node_t *p_p_node;

	count = 0;

	for(i = 0, p_p_node = p_array->p_nodes;
	    i < p_array->num;
	    i++, p_p_node++) {
		if(*p_p_node != NULL) {
			count += ((*p_p_node)->scale - (*p_p_node)->remains);
		}
	}

	return count;
}
