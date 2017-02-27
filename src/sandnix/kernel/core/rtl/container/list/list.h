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

#include "./list_defs.h"

#include "../container_defs.h"

//Initialize
#define core_rtl_list_init(p_list)		(*(p_list) = NULL)

//Check if the list is empty
#define core_rtl_list_empty(p_list)		(*(p_list) == NULL)

//Insert afer pos
//Return the position of the new item
plist_node_t core_rtl_list_insert_before(
    plist_node_t pos,	//Position. If NULL, insert in the front of the list.
    plist_t p_list,		//Link list
    void* p_item,		//Item
    pheap_t heap);		//Heap

plist_node_t core_rtl_list_insert_node_before(
    plist_node_t pos,		//Position. If NULL, insert in the front of the list.
    plist_t p_list,			//Link list
    plist_node_t p_node);	//Node tt insert

//Insert before pos
//Return the position of the new item
plist_node_t core_rtl_list_insert_after(
    plist_node_t pos,	//If NULL, insert at the end of the list.
    plist_t p_list,
    void* p_item,
    pheap_t heap);

plist_node_t core_rtl_list_insert_node_after(
    plist_node_t pos,	//If NULL, insert at the end of the list.
    plist_t p_list,
    plist_node_t p_node);

//Remove
void* core_rtl_list_remove(
    plist_node_t pos,	//Position
    plist_t p_list,		//List
    pheap_t heap);		//Heap

void* core_rtl_list_node_remove(
    plist_node_t pos,	//Position
    plist_t p_list);	//List

//Destroy
void core_rtl_list_destroy(
    plist_t p_list,				//List
    pheap_t heap,				//Heap
    item_destroyer_t destroier,	//The callback to destroy items
    void* arg);					//The arguments of the callback

//Join
void core_rtl_list_join(
    plist_t p_src,		//Source
    plist_t p_dest,		//Destination
    pheap_t src_heap,	//Heap of sourve
    pheap_t dest_heap);	//Heap of dest

//Get prev node
#define core_rtl_list_prev(pos, p_list)		(((pos) == *(p_list)) \
        ? NULL \
        : ((pos)->p_prev))

//Get next node
#define core_rtl_list_next(pos, p_list)		((pos == (*(p_list))->p_prev) \
        ? NULL \
        : ((pos)->p_next))

//Get item
#define core_rtl_list_get(pos)				((pos)->p_item)

//Quick sort
void core_rtl_list_qsort(
    plist_t p_list,			//List
    item_compare_t compare,	//Callback to compare items
    bool b2s);				//If true, in ascending order, if false in descending order
