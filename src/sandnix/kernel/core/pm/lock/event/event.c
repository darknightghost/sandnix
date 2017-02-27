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

#include "./event.h"
#include "../../pm.h"
#include "../../../rtl/rtl.h"
#include "../../../exception/exception.h"

void core_pm_event_init(pevent_t p_event)
{
    core_pm_spnlck_init(&(p_event->lock));
    p_event->ticket = 0;
    p_event->wake_up_before = 0;
    p_event->alive = true;
    core_rtl_list_init(&(p_event->wait_list));

    return;
}

kstatus_t core_pm_event_wait(pevent_t p_event, s32 millisec_timeout)
{
    core_pm_spnlck_lock(&(p_event->lock));

    //Test if the event is alive
    if(!p_event->alive) {
        core_pm_spnlck_unlock(&(p_event->lock));
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal event.");
        return EINVAL;
    }

    event_wait_thrd_info_t wait_info = {
        .ticket = p_event->ticket,
        .thread_id = core_pm_get_currnt_thread_id()
    };

    list_node_t node = {
        .p_item = &wait_info
    };

    //Add to wait list
    core_rtl_list_insert_node_after(NULL, &(p_event->wait_list), &node);

    //Wait for event
    if(millisec_timeout >= 0) {
        //Sleep
        core_pm_disable_sched();
        u64 ns = (u64)millisec_timeout * 1000 * 1000;
        core_pm_sleep(&ns);
        core_pm_spnlck_unlock(&(p_event->lock));
        core_pm_enable_sched();
        core_pm_schedule();

    } else {
        //Suspend
        core_pm_disable_sched();
        core_pm_suspend(wait_info.thread_id);
        core_pm_spnlck_unlock(&(p_event->lock));
        core_pm_enable_sched();
        core_pm_schedule();

    }

    core_pm_spnlck_lock(&(p_event->lock));

    //Remove current thread from wait list
    core_rtl_list_node_remove(&node, &(p_event->wait_list));

    if(!p_event->alive) {
        //Owner dead
        core_pm_spnlck_unlock(&(p_event->lock));
        core_exception_set_errno(EOWNERDEAD);
        return EOWNERDEAD;

    } else if((p_event->ticket > wait_info.ticket
               && p_event->wake_up_before > wait_info.ticket)
              || (p_event->ticket < wait_info.ticket
                  && p_event->wake_up_before > wait_info.ticket)
              || (p_event->ticket < wait_info.ticket
                  && p_event->wake_up_before > wait_info.ticket)) {
        //Return success
        core_pm_spnlck_unlock(&(p_event->lock));
        core_exception_set_errno(ESUCCESS);
        return ESUCCESS;

    } else {
        //Try again
        core_pm_spnlck_unlock(&(p_event->lock));
        core_exception_set_errno(EAGAIN);
        return EAGAIN;
    }
}

void core_pm_event_set(pevent_t p_event, bool broadcast)
{
    core_pm_spnlck_lock(&(p_event->lock));

    //Test if the event is alive
    if(!p_event->alive) {
        core_pm_spnlck_unlock(&(p_event->lock));
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal event.");
        return;
    }

    if(core_rtl_list_empty(&(p_event->wait_list))) {
        core_pm_spnlck_unlock(&(p_event->lock));
        core_exception_set_errno(ESUCCESS);
        return;
    }

    //Set ticket value to wakeup
    if(broadcast) {
        p_event->wake_up_before = p_event->ticket;

    } else {
        p_event->wake_up_before =
            ((pevent_wait_thrd_info_t)(p_event->wait_list->p_item))
            ->ticket + 1;
    }

    //Wakeup threads
    plist_node_t p_node = p_event->wait_list;

    core_pm_disable_sched();

    do {
        pevent_wait_thrd_info_t p_info = (pevent_wait_thrd_info_t)(p_node->p_item);

        if(p_info->ticket >= p_event->wake_up_before) {
            break;
        }

        core_pm_resume(p_info->thread_id);

        p_node = p_node->p_next;
    } while(p_node != p_event->wait_list);

    core_pm_spnlck_unlock(&(p_event->lock));
    core_pm_enable_sched();
    core_pm_schedule();

    core_exception_set_errno(ESUCCESS);
    return;
}

void core_pm_event_destroy(pevent_t p_event)
{
    core_pm_spnlck_lock(&(p_event->lock));

    //Test if the event is alive
    if(!p_event->alive) {
        core_pm_spnlck_unlock(&(p_event->lock));
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal event.");
        return;
    }

    p_event->alive = false;

    //Wakeup threads
    plist_node_t p_node = p_event->wait_list;

    core_pm_disable_sched();

    do {
        pevent_wait_thrd_info_t p_info = (pevent_wait_thrd_info_t)(p_node->p_item);
        core_pm_resume(p_info->thread_id);
        p_node = p_node->p_next;
    } while(p_node != p_event->wait_list);

    core_pm_spnlck_unlock(&(p_event->lock));
    core_pm_enable_sched();
    core_pm_schedule();

    core_exception_set_errno(ESUCCESS);
    return;
}
