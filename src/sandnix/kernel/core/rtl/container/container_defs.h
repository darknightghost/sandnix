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

#include "../../../../../common/common.h"

//Call back functions
//Destroy item
//void item_destroyer(void* p_item, void* p_arg)
typedef void (*item_destroyer_t)(void*, void*);

//Compare two item.If item1 > item2, the return value > 0.
//If the two item equals, return 0.
//If item1 < item2, the return value < 0.
//int item_compare(void* p_item1, void* p_item2);
typedef int (*item_compare_t)(void*, void*);

#include "./map/map_defs.h"
#include "./array/array_defs.h"
#include "./queue/queue_defs.h"
#include "./list/list_defs.h"
#include "./stack/stack_defs.h"
