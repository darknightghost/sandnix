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

#ifndef	CORE_PM_EXPORT
    #include "../../rtl/rtl.h"
    #include "../../mm/mm.h"
#endif

typedef	struct	_obj	obj_t, *pobj_t;

typedef struct	_thread_ref_obj {
    obj_t		obj;
    u32			thread_id;		//Which thread the object belongs to

    //Called when the thread exit
    void	(*on_thread_exit)(struct _thread_ref_obj*);
} thread_ref_obj_t, *pthread_ref_obj_t;

//Construction function
pthread_ref_obj_t thread_ref_obj(u32 thread_id,
                                 void (*on_thread_exit)(pthread_ref_obj_t),
                                 u32 class_id,
                                 destructor_t destructor,
                                 compare_obj_t compare_func,
                                 to_string_t to_string_func,
                                 pheap_t heap, size_t size);

