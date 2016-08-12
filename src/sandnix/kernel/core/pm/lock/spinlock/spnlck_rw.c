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

#include "spnlck_rw.h"
#include "../../thread/thread.h"
#include "../../../../hal/rtl/rtl.h"
#include "../../../../hal/exception/exception.h"

void core_pm_spnlck_rw_init(pspnlck_rw_t p_lock)
{
    core_pm_spnlck_init(&(p_lock->lock));
    p_lock->reader_count = 0;
    return;
}

void core_pm_spnlck_rw_r_lock(pspnlck_rw_t p_lock)
{
    core_pm_spnlck_lock(&(p_lock->lock));

    hal_rtl_atomic_addl(p_lock->reader_count, 1);
    core_pm_spnlck_unlock(&(p_lock->lock));
    return;
}

kstatus_t core_pm_spnlck_rw_r_trylock(pspnlck_rw_t p_lock)
{
    kstatus_t status = core_pm_spnlck_trylock(&(p_lock->lock));

    if(status != ESUCCESS) {
        return status;
    }

    hal_rtl_atomic_addl(p_lock->reader_count, 1);
    core_pm_spnlck_unlock(&(p_lock->lock));
    return ESUCCESS;
}

void core_pm_spnlck_rw_r_unlock(pspnlck_rw_t p_lock)
{
    hal_rtl_atomic_subl(p_lock->reader_count, 1);
    return;
}

void core_pm_spnlck_rw_w_lock(pspnlck_rw_t p_lock)
{
    core_pm_spnlck_lock(&(p_lock->lock));

    while(p_lock->reader_count > 0) {
        MEM_BLOCK;
    }

    return;
}

kstatus_t core_pm_spnlck_rw_w_trylock(pspnlck_rw_t p_lock)
{
    kstatus_t status = core_pm_spnlck_trylock(&(p_lock->lock));

    if(status != ESUCCESS) {
        return status;
    }

    if(p_lock->reader_count == 0) {
        return ESUCCESS;

    } else {
        core_pm_spnlck_unlock(&(p_lock->lock));
        return EAGAIN;
    }
}

void core_pm_spnlck_rw_w_unlock(pspnlck_rw_t p_lock)
{
    core_pm_spnlck_unlock(&(p_lock->lock));
    return;
}

