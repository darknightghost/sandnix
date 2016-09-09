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

#include "queue.h"

void core_rtl_queue_init(pqueue_t p_queue, pheap_t heap)
{
    p_queue->heap = heap;
    p_queue->data_list = NULL;
    return;
}

bool core_rtl_queue_push(pqueue_t p_queue, void* p_item)
{
    if(core_rtl_list_insert_after(NULL,
                                  &(p_queue->data_list),
                                  p_item,
                                  p_queue->heap) != NULL) {
        return true;
    }

    return false;
}

void* core_rtl_queue_pop(pqueue_t p_queue)
{
    if(p_queue->data_list == NULL) {
        return NULL;
    }

    void* p_ret = p_queue->data_list->p_item;
    core_rtl_list_remove(p_queue->data_list,
                         &(p_queue->data_list),
                         p_queue->heap);

    return p_ret;
}

void* core_rtl_queue_front(pqueue_t p_queue)
{
    if(p_queue->data_list == NULL) {
        return NULL;
    }

    return p_queue->data_list->p_item;
}

void* core_rtl_queue_end(pqueue_t p_queue)
{
    if(p_queue->data_list == NULL) {
        return NULL;
    }

    return p_queue->data_list->p_prev->p_item;
}

void core_rtl_queue_destroy(pqueue_t p_queue, item_destroyer_t destroier,
                            void* arg)
{
    core_rtl_list_destroy(&(p_queue->data_list), p_queue->heap, destroier, arg);
    return;
}
