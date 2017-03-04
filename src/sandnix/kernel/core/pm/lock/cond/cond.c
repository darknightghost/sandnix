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

#include "../../../../../../common/common.h"
#include "./cond.h"
#include "../../pm.h"
#include "../../../rtl/rtl.h"
#include "../../../exception/exception.h"


void core_pm_cond_init(pcond_t p_cond, pmutex_t p_mutex)
{
    p_cond->p_lock = p_mutex;
    p_cond->ticket = 0;
    p_cond->wake_up_before = 0;
    p_cond->alive = true;
    core_rtl_list_init(&(p_cond->wait_list));
}

kstatus_t core_pm_cond_wait(pcond_t p_cond, s32 millisec_timeout)
{
    //Test if mutex got
    if(!core_pm_mutex_got(p_cond->p_lock)) {
        peperm_except_t p_except = eperm_except();
        RAISE(p_except, "Mutex status error!");

        return EPERM;
    }

    //Test if the cond is alive
    if(!p_cond->alive) {
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal cond.");
        return EINVAL;
    }

    cond_wait_thrd_info_t wait_info = {
        .ticket = p_cond->ticket,
        .thread_id = core_pm_get_currnt_thread_id()
    };

    list_node_t node = {
        .p_item = &wait_info
    };

    //Add to wait list
    core_rtl_list_insert_node_after(NULL, &(p_cond->wait_list), &node);

    //Wait for cond
    if(millisec_timeout >= 0) {
        //Sleep
        core_pm_disable_sched();
        u64 ns = (u64)millisec_timeout * 1000 * 1000;
        core_pm_sleep(&ns);
        core_pm_mutex_release(p_cond->p_lock);

    } else {
        //Suspend
        core_pm_disable_sched();
        core_pm_suspend(wait_info.thread_id);
        core_pm_mutex_release(p_cond->p_lock);

    }

    for(kstatus_t status = core_pm_mutex_acquire(p_cond->p_lock, -1);
        status != ESUCCESS;
        status = core_pm_mutex_acquire(p_cond->p_lock, -1)) {
        if(status == EOWNERDEAD) {
            peownerdead_except_t p_except = eownerdead_except();
            RAISE(p_except, "Dead mutex");
            break;
        }
    }


    //Remove current thread from wait list
    core_rtl_list_node_remove(&node, &(p_cond->wait_list));

    if(!p_cond->alive) {
        //Owner dead
        core_exception_set_errno(EOWNERDEAD);
        return EOWNERDEAD;

    } else if((p_cond->ticket > wait_info.ticket
               && p_cond->wake_up_before > wait_info.ticket)
              || (p_cond->ticket < wait_info.ticket
                  && p_cond->wake_up_before > wait_info.ticket)
              || (p_cond->ticket < wait_info.ticket
                  && p_cond->wake_up_before > wait_info.ticket)) {
        //Return success
        core_exception_set_errno(ESUCCESS);
        return ESUCCESS;

    } else {
        //Try again
        core_exception_set_errno(EAGAIN);
        return EAGAIN;
    }
}

kstatus_t core_pm_cond_signal(pcond_t p_cond, bool broadcast)
{
    bool locked = core_pm_mutex_got(p_cond->p_lock);

    if(!locked) {
        for(kstatus_t status = core_pm_mutex_acquire(p_cond->p_lock, -1);
            status != ESUCCESS;
            status = core_pm_mutex_acquire(p_cond->p_lock, -1)) {
            if(status == EOWNERDEAD) {
                peownerdead_except_t p_except = eownerdead_except();
                RAISE(p_except, "Dead mutex");
                break;
            }
        }
    }

    //Test if the cond is alive
    if(!p_cond->alive) {
        if(!locked) {
            core_pm_mutex_release(p_cond->p_lock);
        }

        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal cond.");
        return EINVAL;
    }

    if(core_rtl_list_empty(&(p_cond->wait_list))) {
        if(!locked) {
            core_pm_mutex_release(p_cond->p_lock);
        }

        core_exception_set_errno(ESUCCESS);
        return ESUCCESS;
    }

    //Set ticket value to wakeup
    if(broadcast) {
        p_cond->wake_up_before = p_cond->ticket;

    } else {
        p_cond->wake_up_before =
            ((pcond_wait_thrd_info_t)(p_cond->wait_list->p_item))
            ->ticket + 1;
    }

    //Wakeup threads
    plist_node_t p_node = p_cond->wait_list;

    core_pm_disable_sched();

    do {
        pcond_wait_thrd_info_t p_info = (pcond_wait_thrd_info_t)(p_node->p_item);

        if(p_info->ticket >= p_cond->wake_up_before) {
            break;
        }

        core_pm_resume(p_info->thread_id);

        p_node = p_node->p_next;
    } while(p_node != p_cond->wait_list);

    if(!locked) {
        core_pm_mutex_release(p_cond->p_lock);

    } else {
        core_pm_enable_sched();
        core_pm_schedule();
    }

    core_exception_set_errno(ESUCCESS);
    return ESUCCESS;
}

void core_pm_cond_destroy(pcond_t p_cond)
{
    bool locked = core_pm_mutex_got(p_cond->p_lock);

    if(!locked) {
        for(kstatus_t status = core_pm_mutex_acquire(p_cond->p_lock, -1);
            status != ESUCCESS;
            status = core_pm_mutex_acquire(p_cond->p_lock, -1)) {
            if(status == EOWNERDEAD) {
                peownerdead_except_t p_except = eownerdead_except();
                RAISE(p_except, "Dead mutex");
                break;
            }
        }
    }

    //Test if the cond is alive
    if(!p_cond->alive) {
        if(!locked) {
            core_pm_mutex_release(p_cond->p_lock);
        }

        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal cond.");
        return;
    }

    p_cond->alive = false;

    //Wakeup threads
    plist_node_t p_node = p_cond->wait_list;

    core_pm_disable_sched();

    do {
        pcond_wait_thrd_info_t p_info = (pcond_wait_thrd_info_t)(p_node->p_item);
        core_pm_resume(p_info->thread_id);
        p_node = p_node->p_next;
    } while(p_node != p_cond->wait_list);

    if(!locked) {
        core_pm_mutex_release(p_cond->p_lock);

    } else {
        core_pm_enable_sched();
        core_pm_schedule();
    }

    core_exception_set_errno(ESUCCESS);
    return;
}
