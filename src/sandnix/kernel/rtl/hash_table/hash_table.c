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

#include "hash_table.h"
#include "../../mm/mm.h"
#include "../../pm/pm.h"
#include "../../exceptions/exceptions.h"

static	k_status	add_index(list_t* p_list, void* index, void* p_item, void* heap);
static	void		list_destroyer(list_t p_list, void* heap);
static	void		node_destroyer(phash_table_index_node_t p_node, void* heap);

k_status	rtl_hash_table_init(phash_table_t p_table,
                                u32 base,
                                u32 max,
                                hash_func_t hash_func,
                                compare_func_t compare_func,
                                void* heap)
{
	p_table->base = base;
	p_table->max = max;
	p_table->hash_func = hash_func;
	p_table->compare_func = compare_func;
	rtl_array_list_init(&(p_table->table), max - base + 1, heap);
	return ESUCCESS;
}

k_status rtl_hash_table_set(phash_table_t p_table,
                            void* index,
                            void* p_item,
                            void* heap)
{
	u32 hash;
	list_t list;
	phash_table_index_node_t p_node;
	plist_node_t p_lst_node;
	k_status status;

	hash = p_table->hash_func(index);

	list = rtl_array_list_get(&(p_table->table), hash - p_table->base);

	p_lst_node = list;

	//Search for the index
	if(p_lst_node != NULL) {
		do {
			p_node = p_lst_node->p_item;

			if(p_table->compare_func(p_node->index, index)) {
				p_node->p_item = p_item;
				pm_set_errno(ESUCCESS);
				return ESUCCESS;
			}
		} while(p_lst_node != list);

	}

	//Create new index
	status = add_index(&list, index, p_item, heap);

	if(status != ESUCCESS) {
		return status;
	}

	return rtl_array_list_set(&(p_table->table),
	                          hash - p_table->base,
	                          p_node,
	                          heap);
}

void* rtl_hash_table_get(phash_table_t p_table,
                         void* index)
{
	u32 hash;
	list_t list;
	phash_table_index_node_t p_node;
	plist_node_t p_lst_node;


	hash = p_table->hash_func(index);

	list = rtl_array_list_get(&(p_table->table), hash - p_table->base);

	if(list == NULL) {
		pm_set_errno(ESUCCESS);
		return NULL;
	}

	//Search for index
	p_lst_node = list;

	do {
		p_node = p_lst_node->p_item;

		if(p_table->compare_func(p_node->index, index)) {
			pm_set_errno(ESUCCESS);
			return p_node->p_item;
		}
	} while(p_lst_node != list);

	pm_set_errno(ESUCCESS);
	return NULL;
}

void rtl_hash_table_remove(phash_table_t p_table,
                           void* index,
                           void* heap)
{
	u32 hash;
	list_t list;
	phash_table_index_node_t p_node;
	plist_node_t p_lst_node;


	hash = p_table->hash_func(index);

	list = rtl_array_list_get(&(p_table->table), hash - p_table->base);

	if(list == NULL) {
		pm_set_errno(EINVAL);
		return;
	}

	//Search for index
	p_lst_node = list;

	do {
		p_node = p_lst_node->p_item;

		if(p_table->compare_func(p_node->index, index)) {
			rtl_list_remove(&list, p_lst_node, heap);

			if(list == NULL) {
				rtl_array_list_release(&(p_table->table),
				                       hash - p_table->base,
				                       heap);
			}

			pm_set_errno(ESUCCESS);
			return;
		}
	} while(p_lst_node != list);

	pm_set_errno(EINVAL);
	return;
}

void rtl_hash_table_destroy(phash_table_t p_table,
                            void* heap)
{
	rtl_array_list_destroy(&(p_table->table),
	                       (item_destroyer_callback)list_destroyer,
	                       heap,
	                       heap);
	return;
}

k_status add_index(list_t* p_list, void* index, void* p_item, void* heap)
{
	phash_table_index_node_t p_node;

	//Create new node
	p_node = mm_hp_alloc(sizeof(hash_table_index_node_t),
	                     heap);

	if(p_node == NULL) {
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	p_node->index = index;
	p_node->p_item = p_item;

	if(rtl_list_insert_after(p_list, NULL, p_node, heap) == NULL) {
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	return ESUCCESS;
}

void list_destroyer(list_t p_list, void* heap)
{
	rtl_list_destroy(&p_list,
	                 heap,
	                 (item_destroyer_callback)node_destroyer,
	                 heap);
	return;
}

void node_destroyer(phash_table_index_node_t p_node, void* heap)
{
	mm_hp_free(p_node, heap);
	return;
}
