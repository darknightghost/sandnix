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
#include "../container.h"
#include "../../../mm/mm.h"

typedef struct	_queue {
    list_t		data_list;
    pheap_t		heap;
} queue_t, *pqueue_t;

//Initialize
void core_rtl_queue_init(
    pqueue_t p_queue,
    pheap_t heap);

//Push item
bool core_rtl_queue_push(
    pqueue_t p_queue,
    void* p_item);

//Pop item
void* core_rtl_queue_pop(
    pqueue_t p_queue);

//Get first item
void* core_rtl_queue_front(
    pqueue_t p_queue);

//Get last item
void* core_rtl_queue_end(
    pqueue_t p_queue);

//Destroy item
void core_rtl_queue_destroy(
    pqueue_t p_queue,
    item_destroyer_t destroier,
    void* arg);
