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

#ifndef	LIST_H_INCLUDE
#define	LIST_H_INCLUDE

#include "../../../common/common.h"

typedef		void	(*item_destroyer_callback)(void*);

typedef	struct _list_node {
	struct _list_node*	p_prev;
	struct _list_node*	p_next;
	void*				p_item;
} list_node, *plist_node, *list;

plist_node		rtl_list_insert_before(
    list* p_list,
    plist_node position,		//Null if at the start of the list
    void* p_item,
    void* heap);
plist_node		rtl_list_insert_after(
    list* p_list,
    plist_node position,		//Null if at the end of the list
    void* p_item,
    void* heap);
void		rtl_list_remove(list* p_list, plist_node p_node, void* heap);
plist_node	rtl_list_get_node_by_item(list lst, void* p_item);
void		rtl_list_destroy(list* p_list, void* heap, item_destroyer_callback callback);

#endif	//!	LIST_H_INCLUDE
