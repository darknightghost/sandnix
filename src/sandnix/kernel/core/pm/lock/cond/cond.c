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
#include "../../../mm/mm.h"
#include "../../../rtl/rtl.h"
#include "../../../exception/exception.h"
#include "../../../../hal/rtl/rtl.h"


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

    u32 ticket = 1;
    hal_rtl_atomic_xaddl(p_cond->ticket, ticket);
    cond_wait_thrd_info_t wait_info = {
        .list_node.p_item = &wait_info,
        .ticket = ticket,
        .thread_id = core_pm_get_currnt_thread_id()
    };

    //Add to wait list
    core_rtl_list_insert_node_after(NULL, &(p_cond->wait_list), &(wait_info.list_node));

    //Wait for cond
    if(millisec_timeout >= 0) {
        //Sleep
        core_pm_disable_sched();
        u64 ns = (u64)millisec_timeout * 1000 * 1000;
        core_pm_sleep(&ns);

    } else {
        //Suspend
        core_pm_disable_sched();
        core_pm_suspend(wait_info.thread_id);
    }

    core_pm_mutex_release(p_cond->p_lock);

    core_pm_enable_sched();
    core_pm_schedule();

    for(kstatus_t status = core_pm_mutex_acquire(p_cond->p_lock, -1);
        status != ESUCCESS;
        status = core_pm_mutex_acquire(p_cond->p_lock, -1)) {
        if(status == EOWNERDEAD) {
            core_rtl_list_node_remove(&(wait_info.list_node), &(p_cond->wait_list));
            peownerdead_except_t p_except = eownerdead_except();
            RAISE(p_except, "Dead mutex");
            break;
        }
    }


    //Remove current thread from wait list
    core_rtl_list_node_remove(&(wait_info.list_node), &(p_cond->wait_list));

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

kstatus_t core_pm_cond_multi_wait(pcond_t* conds, size_t num, u32* p_ret_indexs,
                                  size_t* p_ret_num, s32 millisec_timeout,
                                  pheap_t heap)
{
    //Get locks
    pmutex_t* locks = (pmutex_t*)core_mm_heap_alloc(sizeof(pmutex_t) * num, heap);
    size_t lock_num = 0;
    pexcept_obj_t p_except = NULL;
    char* except_comment = NULL;
    kstatus_t status = ESUCCESS;

    if(locks == NULL) {
        status = ENOMEM;
        p_except = (pexcept_obj_t)enomem_except();
        except_comment = "Failed to allocate memory for mutex list.";
        goto ALLOCATE_LOCKS_ERROR;
    }

    for(u32 i = 0; i < num; i++) {
        bool appeared = false;

        for(u32 j = 0; j < i; j++) {
            if(conds[i]->p_lock == conds[j]->p_lock) {
                appeared = true;
                break;
            }
        }

        if(!appeared) {
            locks[lock_num] = conds[i]->p_lock;
            lock_num++;
        }
    }

    //Allocate memory for wait informations
    pcond_wait_thrd_info_t wait_thrd_infos =
        (pcond_wait_thrd_info_t)core_mm_heap_alloc(
            sizeof(cond_wait_thrd_info_t) * num,
            heap);

    if(wait_thrd_infos == NULL) {
        status = ENOMEM;
        p_except = (pexcept_obj_t)enomem_except();
        except_comment = "Failed to allocate memory for waiting thread informations.";
        goto ALLOCATE_WAIT_INFO_ERROR;
    }

    //Test locks
    for(u32 i = 0; i < lock_num; i++) {
        if(!core_pm_mutex_got(locks[i])) {
            p_except = (pexcept_obj_t)eperm_except();
            except_comment = "Mutex status error!";

            goto TEST_LOCK_FAILED_ERROR;
        }
    }

    //Test cond status
    for(u32 i = 0; i < num; i++) {
        if(!conds[i]->alive) {
            p_except = (pexcept_obj_t)einval_except();
            except_comment = "Illegal cond.";
            status = EINVAL;
            goto TEST_COND_STAT_ERROR;
        }
    }

    //Add to wait list
    u32 current_thrd_id = core_pm_get_currnt_thread_id();

    for(u32 i = 0; i < num; i++) {
        u32 ticket = 1;
        hal_rtl_atomic_xaddl(conds[i]->ticket, ticket);
        wait_thrd_infos[i].list_node.p_item = &wait_thrd_infos[i];
        wait_thrd_infos[i].ticket = ticket;
        wait_thrd_infos[i].thread_id = current_thrd_id;

        //Add to wait list
        core_rtl_list_insert_node_after(NULL,
                                        &(conds[i]->wait_list),
                                        &(wait_thrd_infos[i].list_node));
    }

    //Wait for conds
    if(millisec_timeout >= 0) {
        //Sleep
        core_pm_disable_sched();
        u64 ns = (u64)millisec_timeout * 1000 * 1000;
        core_pm_sleep(&ns);

    } else {
        //Suspend
        core_pm_disable_sched();
        core_pm_suspend(current_thrd_id);
    }

    //Unlock all mutexes
    for(u32 i = 0; i < lock_num; i++) {
        core_pm_mutex_release(locks[i]);
    }

    core_pm_enable_sched();
    core_pm_schedule();

    //Lock locks
    for(u32 i = 0; i < lock_num; i++) {
        for(status = core_pm_mutex_acquire(locks[i], -1);
            status != ESUCCESS;
            status = core_pm_mutex_acquire(locks[i], -1)) {
            if(status == EOWNERDEAD) {
                for(u32 j = 0; j < i; j++) {
                    core_pm_mutex_release(locks[j]);
                }

                p_except = (pexcept_obj_t)eownerdead_except();
                except_comment = "Dead mutex";
                goto ACQUIRE_MUTEXES_ERROR;
            }
        }
    }

    //Get result
    *p_ret_num = 0;
    bool has_dead = false;

    for(u32 i = 0; i < num; i++) {
        if(!conds[i]->alive) {
            has_dead = true;

        } else if((conds[i]->ticket > wait_thrd_infos[i].ticket
                   && conds[i]->wake_up_before > wait_thrd_infos[i].ticket)
                  || (conds[i]->ticket < wait_thrd_infos[i].ticket
                      && conds[i]->wake_up_before > wait_thrd_infos[i].ticket)
                  || (conds[i]->ticket < wait_thrd_infos[i].ticket
                      && conds[i]->wake_up_before > wait_thrd_infos[i].ticket)) {
            //Add result
            p_ret_indexs[*p_ret_num] = i;
            (*p_ret_num)++;
        }
    }

    //Remove current thread from wait lists
    for(u32 i = 0; i < num; i++) {
        core_rtl_list_node_remove(
            &(wait_thrd_infos[i].list_node),
            &(conds[i]->wait_list));
    }

    if(*p_ret_num > 0) {
        status = ESUCCESS;

    } else {
        if(has_dead) {
            status = EOWNERDEAD;

        } else {
            status = EAGAIN;
        }
    }

    core_mm_heap_free(locks, heap);
    core_mm_heap_free(wait_thrd_infos, heap);
    core_exception_set_errno(status);
    return status;

    //Exception handlers
ACQUIRE_MUTEXES_ERROR:

    for(u32 i = 0; i < num; i++) {
        core_rtl_list_node_remove(
            &(wait_thrd_infos[i].list_node),
            &(conds[i]->wait_list));
    }

TEST_COND_STAT_ERROR:
TEST_LOCK_FAILED_ERROR:
    core_mm_heap_free(wait_thrd_infos, heap);

ALLOCATE_WAIT_INFO_ERROR:
    core_mm_heap_free(locks, heap);

ALLOCATE_LOCKS_ERROR:
    core_exception_set_errno(status);

    if(p_except != NULL) {
        RAISE(p_except, except_comment);
    }

    return status;
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
