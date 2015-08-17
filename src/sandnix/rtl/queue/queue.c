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

#include "queue.h"

void rtl_queue_init(pqueue_t p_q)
{
	p_q->data_list = NULL;
	p_q->data_num = 0;
	return;
}

bool rtl_queue_push(pqueue_t p_q, void* p_item, void* heap)
{
	if(rtl_list_insert_after(
	       &(p_q->data_list),
	       NULL,
	       p_item,
	       heap) == NULL) {
		return false;
	}

	(p_q->data_num)++;
	return true;
}

void* rtl_queue_pop(pqueue_t p_q, void* heap)
{
	void* ret;

	if(p_q->data_num == 0) {
		return NULL;

	} else {
		ret = p_q->data_list->p_item;
		rtl_list_remove(&(p_q->data_list), p_q->data_list, heap);
		(p_q->data_num)--;
		return ret;
	}
}

u32 rtl_queue_size(pqueue_t p_q)
{
	return p_q->data_num;
}

void* rtl_queue_front(pqueue_t p_q)
{
	if(p_q->data_num == 0) {
		return NULL;

	} else {
		return p_q->data_list->p_item;
	}
}

void* rtl_queue_back(pqueue_t p_q)
{
	if(p_q->data_num == 0) {
		return NULL;

	} else {
		return p_q->data_list->p_prev->p_item;
	}
}
