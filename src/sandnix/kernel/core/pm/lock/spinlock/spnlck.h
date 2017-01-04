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

#include "./spnlck_defs.h"

//Initialize the lock.
void	core_pm_spnlck_init(pspnlck_t p_lock);

//Lock(increase the priority of current thread).
void	core_pm_spnlck_lock(pspnlck_t p_lock);

//Lock
void	core_pm_spnlck_raw_lock(pspnlck_t p_lock);

//Try lock(increase the priority of current thread).
kstatus_t	core_pm_spnlck_trylock(pspnlck_t p_lock);

//Try lock
kstatus_t	core_pm_spnlck_raw_trylock(pspnlck_t p_lock);

//Unclock(decrease the priority of current thread)
void	core_pm_spnlck_unlock(pspnlck_t p_lock);

//Unlock
void	core_pm_spnlck_raw_unlock(pspnlck_t p_lock);
