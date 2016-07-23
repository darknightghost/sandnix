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

typedef struct	_list_node {
    struct _list_node*		p_prev;
    struct _list_node*		p_next;
    void*					p_item;
} list_node_t, *plist_node_t, *list_t, **plist_t;

//Initialize
#define core_rtl_list_init(list)

//Check if the list is empty
#define core_rtl_list_empty(list)

//Insert afer pos
plist_node_t core_rtl_list_insert_before(
    plist_node_t pos,	//位置
    plist_t p_list,		//链表地址
    void* p_item,		//元素
    pheap_t heap);		//堆

//Insert before pos
plist_node_t core_rtl_list_insert_after(
    plist_node_t pos,
    plist_t p_list,
    void* p_item,
    pheap_t heap);

//Remove
void core_rtl_list_remove(
    plist_node_t pos,	//位置
    plist_t p_list,		//链表地址
    pheap_t heap);		//堆

//Destroy
void core_rtl_list_destroy(
    plist_t p_list,				//链表
    pheap_t heap,				//堆
    item_destroyer_t destroier,	//销毁元素用的回调函数
    void* arg);					//回调函数额外参数

//Join
void core_rtl_list_join(
    plist_t p_src,		//原链表
    plist_t p_dest,		//目的链表
    pheap_t src_heap,	//原链表所在堆
    pheap_t dest_heap);	//目的链表所在堆

//Get prev node
#define core_rtl_list_prev(pos)

//Get next node
#define core_rtl_list_next(pos)

//Get item
#define core_rtl_list_get(pos)

//Quick sort
void core_rtl_list_qsort(
    plist_t p_list,			//链表
    item_compare_t compare,	//比较大小用的回调函数
    bool b2s);				//true从大到小,false则反之
