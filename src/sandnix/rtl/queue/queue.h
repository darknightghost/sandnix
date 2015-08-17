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

#ifndef	QUEUE_H_INCLUDE
#define	QUEUE_H_INCLUDE

#include "../list/list.h"

typedef	struct	_queue {
	list_t	data_list;
	u32		data_num;
} queue_t, *pqueue_t;

void	rtl_queue_init(pqueue_t p_q);
bool	rtl_queue_push(pqueue_t p_q, void* p_item, void* heap);
void*	rtl_queue_pop(pqueue_t p_q, void* heap);
u32		rtl_queue_size(pqueue_t p_q);
void*	rtl_queue_front(pqueue_t p_q);
void*	rtl_queue_back(pqueue_t p_q);

#endif	//!	QUEUE_H_INCLUDE
