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
#include "./stack_defs.h"
#include "../list/list.h"


//Initialize
#define core_rtl_stack_init(p_stack)	core_rtl_list_init((p_stack))

//Check if the list is empty
#define core_rtl_stack_empty(p_stack)	core_rtl_list_empty((p_stack))

//Push
#define	core_rtl_stack_push(p_stack, p_item, heap) ( \
        core_rtl_list_insert_after( \
                                    NULL, \
                                    (p_stack), \
                                    (p_item), \
                                    (heap)))

//Pop
#define	core_rtl_stack_pop(p_stack, heap) (*(p_stack) == NULL \
        ? NULL \
        : core_rtl_list_remove((*(p_stack))->p_prev, \
                               (p_stack), \
                               (heap)))

//Top
#define	core_rtl_stack_top(p_stack) (*(p_stack) == NULL \
                                     ? NULL \
                                     : (*(p_stack))->p_prev->p_item)

//Destroy
#define	core_rtl_stack_destroy(p_stack, heap, destorier, arg) ( \
        core_rtl_list_destroy( \
                               (p_stack), \
                               (heap), \
                               (destroier), \
                               (arg)))
