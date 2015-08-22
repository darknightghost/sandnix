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

#include "array_list.h"
#include "../../mm/mm.h"
#include "../../pm/pm.h"
#include "../../exceptions/exceptions.h"

static	void	destroy_array_list(array_list_t array);

k_status rtl_array_list_init(array_list_t* p_array,
                             void* heap,
                             size_t size)
{
	return 0;
}

void* rtl_array_list_get(array_list_t array, u32 index, void* heap)
{
	return NULL;
}

k_status rtl_array_list_set(array_list_t array, u32 index, void* value, void* heap)
{
	return 0;
}

void rtl_array_list_release(array_list_t array, u32 index, void* heap);

void destroy_array_list(array_list_t array)
{
	return;
}
