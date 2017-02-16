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
#include "./mutex_defs.h"

//Initialize mutex
void		core_pm_mutex_init(pmutex_t	p_lock, pheap_t heap);

//Acquire mutex
kstatus_t	core_pm_mutex_acquire(pmutex_t p_lock, s32 millisec_timeout);

//Test if current thread got the mutex
bool		core_pm_mutex_got(pmutex_t p_lock);

//Release mutex
void		core_pm_mutex_release(pmutex_t p_lock);

//Destroy mutex
void		core_pm_mutex_destroy(pmutex_t p_lock);
