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

#include "list.h"
#include "../../mm/mm.h"

plist_node rtl_list_insert_before(list* p_list, plist_node position, void* p_item, void* heap)
{
	plist_node p_node, p_new_node;

	//Allocate memory
	p_new_node = mm_hp_alloc(sizeof(list_node), heap);

	if(p_new_node == NULL) {
		return NULL;
	}

	p_new_node->p_item = p_item;

	if(*p_list == NULL) {
		//Insert the new item as the first item
		p_new_node->p_prev = p_new_node;
		p_new_node->p_next = p_new_node;
		*p_list = p_new_node;

	} else if(position == NULL) {
		//Insert the new item as the first item
		p_node = *p_list;
		p_new_node->p_prev = p_node->p_prev;
		p_new_node->p_next = p_node;
		p_node->p_prev->p_next = p_new_node;
		p_node->p_prev = p_new_node;
		*p_list = p_new_node;

	} else {
		//Insert the new item before position
		p_node = position;
		p_new_node->p_prev = p_node->p_prev;
		p_new_node->p_next = p_node;
		p_node->p_prev->p_next = p_new_node;
		p_node->p_prev = p_new_node;

	}

	return p_new_node;
}

plist_node rtl_list_insert_after(list* p_list, plist_node position, void* p_item, void* heap)
{
	plist_node p_node, p_new_node;

	//Allocate memory
	p_new_node = mm_hp_alloc(sizeof(list_node), heap);

	if(p_new_node == NULL) {
		return NULL;
	}

	p_new_node->p_item = p_item;

	if(*p_list == NULL) {
		//Insert the new item as the first item
		p_new_node->p_prev = p_new_node;
		p_new_node->p_next = p_new_node;
		*p_list = p_new_node;

	} else {
		if(position == NULL) {
			p_node = (*p_list)->p_prev;

		} else {
			p_node = position;
		}

		//Insert p_new_node after p_node
		p_new_node->p_next = p_node->p_next;
		p_new_node->p_prev = p_node;
		p_node->p_next->p_prev = p_new_node;
		p_node->p_next = p_new_node;

	}

	return p_new_node;
}

void rtl_list_remove(list* p_list, plist_node p_node, void* heap)
{
	if(*p_list == NULL) {
		return;

	} else if(*p_list == p_node) {
		if(p_node->p_prev == p_node) {
			mm_hp_free(p_node, heap);
			*p_list = NULL;
			return;

		} else {
			*p_list = p_node->p_next;
		}
	}

	p_node->p_prev->p_next = p_node->p_next;
	p_node->p_next->p_prev = p_node->p_prev;
	mm_hp_free(p_node, heap);
	return;

}

plist_node rtl_list_get_node_by_item(list lst, void* p_item)
{
	plist_node p_node;

	if(lst == NULL) {
		return NULL;
	}

	p_node = lst;

	do {
		if(p_node->p_item == p_item) {
			return p_node;
		}

		p_node = p_node->p_next;
	} while(p_node != lst);

	return NULL;
}

void rtl_list_destroy(list* p_list, void* heap, item_destroyer_callback callback)
{
	while(*p_list != NULL) {
		if(callback != NULL) {
			callback((*p_list)->p_item);
		}

		rtl_list_remove(p_list, *p_list, heap);
	}

	return;
}
