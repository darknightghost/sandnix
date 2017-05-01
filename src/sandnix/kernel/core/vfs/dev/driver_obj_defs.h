/*
    Copyright 2017,王思远 <darknightghost.cn@gmail.com>

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
#include "../msg/msg_queue_obj_defs.h"
#include "../../rtl/rtl_defs.h"
#include "../../pm/pm_defs.h"
#include "../file_obj_defs.h"

typedef struct	_driver_obj {
    file_obj_t		file_object;	//Parent file object

    //Member variables
    u32		process_id;			//Which process is the driver process
    u32		uid;				//Driver directory uid
    u32		gid;				//Driver directory gid
    u32		mode;				//Driver directory mode

    list_t	devices;			//Devices list
} driver_obj_t, *pdriver_obj_t;
