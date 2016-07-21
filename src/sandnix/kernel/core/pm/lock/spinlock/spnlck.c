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

#include "spnlck.h"
#include "../../thread/thread.h"
#include "../../../../hal/rtl/rtl.h"

void core_pm_spnlck_init(pspnlck_t p_lock)
{
    p_lock->owner = 0;
    p_lock->ticket = 0;
    p_lock->priority = 0;
    return;
}

void core_pm_spnlck_lock(pspnlck_t p_lock)
{
    u16 ticket;
    u32 thrd_id;
    u32 priority;

    thrd_id = core_pm_get_crrnt_thread_id();

    //Get ticket
    ticket = 1;
    hal_rtl_atomic_xaddw(p_lock->ticket, ticket);

    //Get lock
    priority = core_pm_get_thrd_priority(thrd_id);

    if(priority < PRIORITY_DISPATCH) {
        core_pm_set_thrd_priority(thrd_id, PRIORITY_DISPATCH);
    }

    while(p_lock->owner != ticket) {
        MEM_BLOCK;
    }

    p_lock->priority = priority;

    return;
}

void core_pm_spnlck_raw_lock(pspnlck_t p_lock)
{
    u16 ticket;

    //Get ticket
    ticket = 1;
    hal_rtl_atomic_xaddw(p_lock->ticket, ticket);

    //Get lock
    while(p_lock->owner != ticket) {
        MEM_BLOCK;
    }


    return;
}

kstatus_t core_pm_spnlck_trylock(pspnlck_t p_lock)
{
    u32 thrd_id;
    u32 priority;

    thrd_id = core_pm_get_crrnt_thread_id();
    priority = core_pm_get_thrd_priority(thrd_id);

    if(priority < PRIORITY_DISPATCH) {
        core_pm_set_thrd_priority(thrd_id, PRIORITY_DISPATCH);
    }

    old_lock = p_lock->lock;
    new_lock = old_lock;
    (*((u16*)(&new_lock)))++;

    hal_rtl_atomic_cmpxchgl(p_lock->lock, new_lock, old_lock, result);

    if(!result) {
        core_pm_set_thrd_priority(thrd_id, priority);
        return EAGAIN;

    } else {
        p_lock->priority = priority;
        return ESUCCESS;
    }
}

kstatus_t core_pm_spnlck_raw_trylock(pspnlck_t p_lock)
{
    u32 old_lock;
    u32 new_lock;
    u32 result;

    old_lock = p_lock->lock;
    new_lock = old_lock;
    (*((u16*)(&new_lock)))++;

    hal_rtl_atomic_cmpxchgl(p_lock->lock, new_lock, old_lock, result);

    if(!result) {
        return EAGAIN;

    } else {
        return ESUCCESS;
    }
}

void core_pm_spnlck_unlock(pspnlck_t p_lock)
{
    u32 thrd_id;
    u32 priority;

    thrd_id = core_pm_get_crrnt_thread_id();
    priority = p_lock->priority;

    (p_lock->owner)++;

    core_pm_set_thrd_priority(thrd_id, priority);
    return;
}

void core_pm_spnlck_raw_unlock(pspnlck_t p_lock)
{
    (p_lock->owner)++;
    return;
}
