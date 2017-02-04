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
#include "../../../hal/cpu/cpu_defs.h"
#include "../../rtl/rtl_defs.h"
#include "thread_ref_obj_defs.h"
#include "./thread_defs.h"

typedef	struct _thread_obj {
    obj_t		obj;
    //Basical info
    u32			thread_id;			//Thread id
    u32			status;				//Thread status

    //Process info
    u32			process_id;			//ID of the process of the thread

    //Schedule info
    u32			priority;			//Thread priority
    pcontext_t	p_context;			//Context
    union {
        struct {
            u32			time_slices;		//Number of time slices
        } runing;
        struct {
            u32			awake_tickcount;	//Time to awake
        } sleep;
    } status_info;

    //Referenced objects
    list_t		ref_objs;			//Referenced objects
} thread_obj_t, *pthread_obj_t;
