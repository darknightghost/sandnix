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

#include "stack.h"

bool rtl_stack_push(stack* p_s, void* p_item, void* heap)
{
	return rtl_list_insert_after(
	           p_s,
	           NULL,
	           p_item,
	           heap);
}


void*	rtl_stack_pop(stack* p_s, void* heap)
{
	void* p_ret;

	if(*p_s == NULL) {
		return NULL;
	}

	p_ret = (*p_s)->p_prev->p_item;
	rtl_list_remove(p_s, (*p_s)->p_prev, heap);

	return p_ret;
}
