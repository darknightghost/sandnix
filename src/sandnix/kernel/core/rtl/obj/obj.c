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

#include "obj.h"

void obj(pobj_t p_obj, u32 class_id, destructor_t destructor,
         compare_obj_t compare_func, to_string_t to_string_func,
         pheap_t heap)
{
    p_obj->class_id = class_id;
    p_obj->ref_count = 1;
    p_obj->heap = heap;
    p_obj->destructor = destructor;
    p_obj->compare = compare_func;
    p_obj->to_string = to_string_func;
    return;
}

void core_rtl_obj_inc_ref(pobj_t p_obj)
{
    (p_obj->ref_count)++;
    return;
}

void core_rtl_obj_dec_ref(pobj_t p_obj)
{
    (p_obj->ref_count)--;

    if(p_obj->ref_count == 0) {
        p_obj->destructor(p_obj);
    }

    return;
}
