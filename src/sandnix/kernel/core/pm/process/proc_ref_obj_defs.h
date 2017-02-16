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

#include "../../rtl/obj/obj_defs.h"
struct	_proc_ref_obj;
typedef	struct _proc_ref_obj*		(*proc_ref_obj_fork_t)(struct _proc_ref_obj*, u32);

typedef struct	_proc_ref_obj {
    obj_t		obj;
    u32			process_id;		//Which thread the object belongs to

    struct _proc_ref_obj*	(*fork)(struct _proc_ref_obj*, u32); //Create a copy of object
} proc_ref_obj_t, *pproc_ref_obj_t;
