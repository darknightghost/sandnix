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

#include "mutex.h"
#include "../../../../hal/rtl/rtl.h"
#include "../../../../hal/exception/exception.h"
#include "../../../../hal/cpu/cpu.h"
#include "../../../exception/exception.h"
#include "../../pm.h"

void core_pm_mutex_init(pmutex_t p_lock, pheap_t heap)
{
    core_pm_spnlck_init(&(p_lock->lock));
    p_lock->owner = 0;
    p_lock->lock_flag = false;
    p_lock->alive = true;
    core_rtl_list_init(&(p_lock->wait_list));
    p_lock->heap = heap;

    return;
}

kstatus_t core_pm_mutex_acquire(pmutex_t p_lock, s32 millisec_timeout)
{
    if(!p_lock->alive) {
        core_exception_set_errno(EOWNERDEAD);
        return EOWNERDEAD;
    }

    u32 currnt_thrd = core_pm_get_currnt_thread_id();
    core_pm_disable_sched();
    core_pm_spnlck_lock(&(p_lock->lock));

    if(p_lock->lock_flag == false) {
        //The mutex is free, acquire it now
        p_lock->owner = currnt_thrd;
        p_lock->lock_flag = true;
        core_pm_spnlck_unlock(&(p_lock->lock));
        core_pm_enable_sched();
        core_exception_set_errno(ESUCCESS);
        return ESUCCESS;

    } else {
        if(p_lock->owner == currnt_thrd) {
            //The mutex has been acquired by current thread, raise an exception
            core_pm_spnlck_unlock(&(p_lock->lock));
            core_pm_enable_sched();
            pedeadlock_except_t p_except = edeadlock_except();
            RAISE(p_except, "Mutex has been acquired twice by one thread.");
            return EDEADLK;
        }

        if(millisec_timeout == 0) {
            //Try again
            core_pm_spnlck_unlock(&(p_lock->lock));
            core_pm_enable_sched();
            core_exception_set_errno(EAGAIN);
            return EAGAIN;
        }

        //Add current thread to wait queue
        plist_node_t p_node = core_rtl_list_insert_after(
                                  NULL,
                                  &(p_lock->wait_list),
                                  (void*)currnt_thrd,
                                  p_lock->heap);

        if(millisec_timeout > 0) {
            //Sleep
            u64 ns = (u64)millisec_timeout * 1000 * 1000;
            core_pm_sleep(&ns);

        } else {
            //Suspend thread
            core_pm_suspend(core_pm_get_currnt_thread_id());

        }

        core_pm_spnlck_unlock(&(p_lock->lock));
        core_pm_enable_sched();
        core_pm_schedule();

        //Check if current thread is the owner of mutex
        core_pm_spnlck_lock(&(p_lock->lock));

        if(p_lock->alive) {
            if(p_lock->owner == currnt_thrd) {
                core_pm_spnlck_unlock(&(p_lock->lock));
                core_exception_set_errno(ESUCCESS);
                return ESUCCESS;

            } else {
                //Remove current thread from list
                core_rtl_list_remove(p_node, &(p_lock->wait_list), p_lock->heap);
                core_pm_spnlck_unlock(&(p_lock->lock));
                core_exception_set_errno(EAGAIN);
                return EAGAIN;

            }

        } else {
            //Remove current thread from list
            core_rtl_list_remove(p_node, &(p_lock->wait_list), p_lock->heap);
            core_pm_spnlck_unlock(&(p_lock->lock));
            core_exception_set_errno(EOWNERDEAD);
            return EOWNERDEAD;
        }
    }
}

bool core_pm_mutex_got(pmutex_t p_lock)
{
    bool ret;

    core_pm_spnlck_lock(&(p_lock->lock));

    if(p_lock->alive
       && p_lock->lock_flag
       && p_lock->owner == core_pm_get_currnt_thread_id()) {
        ret = true;

    } else {
        ret = false;
    }

    core_pm_spnlck_unlock(&(p_lock->lock));

    return ret;
}

void core_pm_mutex_release(pmutex_t p_lock)
{
    if(!p_lock->alive) {
        return;
    }

    core_pm_disable_sched();
    core_pm_spnlck_lock(&(p_lock->lock));

    //Check if current thread is the owner of the thread
    if((!p_lock->lock_flag)
       || p_lock->owner != core_pm_get_currnt_thread_id()) {
        core_pm_spnlck_unlock(&(p_lock->lock));
        core_pm_enable_sched();
    }

    if(core_rtl_list_empty(&(p_lock->wait_list))) {
        //Set mutex free
        p_lock->lock_flag = false;

    } else {
        //Switch owner to next thread
        u32 next_thread = (u32)(p_lock->wait_list->p_item);
        core_rtl_list_remove(
            p_lock->wait_list,
            &(p_lock->wait_list),
            p_lock->heap);
        p_lock->owner = next_thread;
        core_pm_resume(p_lock->owner);

    }

    core_pm_spnlck_unlock(&(p_lock->lock));
    core_pm_enable_sched();

    return;
}

void core_pm_mutex_destroy(pmutex_t p_lock)
{
    p_lock->alive = false;
    core_pm_spnlck_lock(&(p_lock->lock));
    plist_node_t p_node = p_lock->wait_list;

    if(p_node != NULL) {
        do {
            core_pm_resume((u32)(p_lock->wait_list->p_item));
        } while(p_node != p_lock->wait_list);
    }

    core_pm_spnlck_unlock(&(p_lock->lock));
    return;
}

