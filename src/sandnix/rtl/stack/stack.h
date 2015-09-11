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

#ifndef	STACK_H_INCLUDE
#define	STACK_H_INCLUDE

#include "../list/list.h"

typedef	struct _list_node	*stack_t;

bool	rtl_stack_push(stack_t* p_s, void* p_item, void* heap);
void*	rtl_stack_pop(stack_t* p_s, void* heap);

#endif	//!	STACK_H_INCLUDE
