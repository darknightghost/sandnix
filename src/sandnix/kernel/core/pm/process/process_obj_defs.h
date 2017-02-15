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
#include "./process_defs.h"
#include "../../rtl/obj/obj_defs.h"
#include "../../rtl/container/map/map_defs.h"

typedef	struct	_process_obj {
    obj_t		obj;				//Base object
    u32			process_id;			//Process id
    u32			exit_code;			//Exit code
    u32			thread_num;			//How many threads does the process have
    map_t		alive_threads;		//Alive threads
    map_t		zombie_threads;		//Zombie threads
    map_t		ref_objs;			//Referenced objects
} process_obj_t, *pprocess_obj_t;
