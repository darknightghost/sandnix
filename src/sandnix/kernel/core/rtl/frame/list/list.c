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

#include "list.h"
#include "../../../mm/mm.h"

static	void	qsort_adjust(plist_node_t begin, plist_node_t end,
                             item_compare_t compare, bool b2s);

plist_node_t core_rtl_list_insert_before(plist_node_t pos, plist_t p_list,
        void* p_item, pheap_t heap)
{
    plist_node_t p_new_node;

    //Allocate new node
    p_new_node = core_mm_heap_alloc(sizeof(list_node_t), heap);

    if(p_new_node == NULL) {
        return NULL;
    }

    if(pos == NULL) {
        //Insert in the front of the list
        if(*p_list == NULL) {
            (*p_list) = p_new_node;
            p_new_node->p_prev = p_new_node;
            p_new_node->p_next = p_new_node;
            p_new_node->p_item = p_item;

        } else {
            p_new_node->p_prev = (*p_list)->p_prev;
            (*p_list)->p_prev->p_next = p_new_node;
            (*p_list)->p_prev = p_new_node;
            p_new_node->p_next = *p_list;
            p_new_node->p_item = p_item;
            *p_list = p_new_node;
        }

    } else {
        p_new_node->p_prev = pos->p_prev;
        pos->p_prev->p_next = p_new_node;
        pos->p_prev = p_new_node;
        p_new_node->p_next = pos;
        p_new_node->p_item = p_item;

        if(pos == *p_list) {
            *p_list = p_new_node;
        }
    }

    return p_new_node;
}

plist_node_t core_rtl_list_insert_after(plist_node_t pos, plist_t p_list,
                                        void* p_item, pheap_t heap)
{
    plist_node_t p_new_node;

    //Allocate new node
    p_new_node = core_mm_heap_alloc(sizeof(list_node_t), heap);

    if(p_new_node == NULL) {
        return NULL;
    }

    if(pos == NULL) {
        //Insert at the end of the list
        if(*p_list == NULL) {
            (*p_list) = p_new_node;
            p_new_node->p_prev = p_new_node;
            p_new_node->p_next = p_new_node;
            p_new_node->p_item = p_item;

        } else {
            p_new_node->p_prev = (*p_list)->p_prev;
            (*p_list)->p_prev->p_next = p_new_node;
            (*p_list)->p_prev = p_new_node;
            p_new_node->p_next = *p_list;
            p_new_node->p_item = p_item;
        }

    } else {
        p_new_node->p_prev = pos;
        p_new_node->p_next = pos->p_next;
        p_new_node->p_prev->p_next = p_new_node;
        p_new_node->p_next->p_prev = p_new_node;
        p_new_node->p_item = p_item;

    }

    return p_new_node;
}

void* core_rtl_list_remove(plist_node_t pos, plist_t p_list, pheap_t heap)
{
    void* ret;

    ret = pos->p_item;

    if(pos->p_prev == pos) {
        *p_list = NULL;

    } else {
        if(*p_list == pos) {
            *p_list = pos->p_next;
        }

        pos->p_prev->p_next = pos->p_next;
        pos->p_next->p_prev = pos->p_prev;
    }

    core_mm_heap_free(pos, heap);

    return ret;
}

void core_rtl_list_destroy(plist_t p_list, pheap_t heap,
                           item_destroyer_t destroier, void* arg)
{
    plist_node_t p_node;
    plist_node_t p_remove_node;

    if(*p_list == NULL) {
        return;
    }

    p_node = *p_list;

    do {
        p_remove_node = p_node;
        p_node = p_node->p_next;

        if(destroier != NULL) {
            destroier(p_remove_node->p_item, arg);
        }

        core_mm_heap_free(p_remove_node, heap);
    } while(p_node != *p_list);

    *p_list = NULL;
    return;
}

void core_rtl_list_join(plist_t p_src, plist_t p_dest, pheap_t src_heap,
                        pheap_t dest_heap)
{
    while(*p_src != NULL) {
        core_rtl_list_insert_after(
            NULL,
            p_dest,
            core_rtl_list_remove(*p_src, p_src, src_heap),
            dest_heap);
    }

    return;
}

void core_rtl_list_qsort(plist_t p_list, item_compare_t compare, bool b2s)
{
    if(*p_list != (*p_list)->p_prev) {
        qsort_adjust(*p_list, (*p_list)->p_prev, compare, b2s);
    }

    return;
}

void qsort_adjust(plist_node_t begin, plist_node_t end,
                  item_compare_t compare, bool b2s)
{
    plist_node_t p;
    plist_node_t p_i;
    plist_node_t p_j;
    void* p_tmp_item;
    int cmp_result;

    p = begin;
    p_i = begin;
    p_j = end;

    while(true) {
        //j
        cmp_result = compare(p->p_item, p_j->p_item);

        if(b2s) {
            cmp_result = -cmp_result;
        }

        if(cmp_result < 0) {
            p_tmp_item = p->p_item;
            p->p_item = p_j->p_item;
            p_j->p_item = p_tmp_item;
            p = p_j;
        }

        if(p_i == p_j) {
            break;
        }

        p_j = p_j->p_prev;

        //i
        cmp_result = compare(p->p_item, p_i->p_item);

        if(b2s) {
            cmp_result = -cmp_result;
        }

        if(cmp_result > 0) {
            p_tmp_item = p->p_item;
            p->p_item = p_i->p_item;
            p_i->p_item = p_tmp_item;
            p = p_i;
        }

        if(p_i == p_j) {
            break;
        }

        p_i = p_i->p_next;

    }

    if(begin != p) {
        qsort_adjust(begin, p, compare, b2s);
    }

    if(end != p) {
        qsort_adjust(p, end, compare, b2s);
    }

    return;
}
