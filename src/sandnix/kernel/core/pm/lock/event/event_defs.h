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

#include "../spinlock/spnlck_defs.h"
#include "../../../rtl/container/list/list_defs.h"
#include "../../../mm/heap/heap_defs.h"

typedef	struct	_event {
    spnlck_t		lock;
    volatile u32	ticket;
    volatile u32	wake_up_before;
    volatile bool	alive;
    list_t			wait_list;
} event_t, *pevent_t;

typedef	struct	_event_wait_thrd_info {
    u32		ticket;
    u32		thread_id;
} event_wait_thrd_info_t, *pevent_wait_thrd_info_t;
