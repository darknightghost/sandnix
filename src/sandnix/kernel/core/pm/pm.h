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

#include "../../../../common/common.h"

#include "./pm_defs.h"

//Process
#include "./process/process.h"

//Thread
#include "./thread/thread.h"

//Spinlock
#include "./lock/spinlock/spnlck.h"
#include "./lock/spinlock/spnlck_rw.h"

//Mutex
#include "./lock/mutex/mutex.h"

//Event
#include "./lock/event/event.h"

//Cond
#include "./lock/cond/cond.h"

void	core_pm_init();
