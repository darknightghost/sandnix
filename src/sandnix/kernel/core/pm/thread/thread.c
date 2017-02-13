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

#include "thread.h"
#include "./thread_obj.h"
#include "../../rtl/rtl.h"
#include "../../mm/mm.h"
#include "../../exception/exception.h"
#include "../../../hal/cpu/cpu.h"
#include "../../../hal/io/io.h"
#include "../../../hal/init/init.h"
#include "../../../hal/rtl/rtl.h"
#include "../pm.h"

//Flag
static	bool				initialized = false;

//Heap
static	pheap_t				sched_heap;

//Schedule list
static	list_t				sched_lists[PRIORITY_HIGHEST + 1];
static	spnlck_t			sched_list_lock;
static	spnlck_t			sched_lock;

//CPU status
static	core_sched_info_t	cpu_infos[MAX_CPU_NUM];
static	u32					runing_cpu_num ;

//Thread table
static	array_t				thread_table;
static	spnlck_rw_t			thread_table_lock;

//Callbacks
static	list_t				thread_ref_callback_list;

static	void				on_tick(u32 int_num, pcontext_t p_context,
                                    u32 err_code);

static	void				add_use_count(pcore_sched_info_t p_info,
        bool is_busy);
static inline void			next_task(pcore_sched_info_t p_info);
static inline void			switch_task(pcore_sched_info_t p_info, u32 cpu_index);
static inline void			reset_timeslices();
static inline void			reset_idle_timeslices();

static inline void			add_ref_obj(u32 thread_id, thread_ref_call_back_t callback);

static void					ipi_preempt(pcontext_t p_context, pipi_arg_obj_t p_null);

void core_pm_thread_init()
{
    //Initialize heap
    sched_heap = core_mm_heap_create(HEAP_MULITHREAD, SANDNIX_KERNEL_PAGE_SIZE);

    if(sched_heap == NULL) {
        PANIC(ENOMEM, "Failed to create schedule heap.");
    }

    //Initialize thread table
    core_rtl_array_init(&thread_table, MAX_THREAD_NUM, sched_heap);
    core_pm_spnlck_rw_init(&thread_table_lock);
    runing_cpu_num = 0;

    core_rtl_list_init(&thread_ref_callback_list);

    //Create thread 0
    pthread_obj_t p_thread_obj = thread_obj_0();
    p_thread_obj->p_node = NULL;
    p_thread_obj->status = TASK_RUNNING;

    if(p_thread_obj == NULL) {
        PANIC(ENOMEM, "Faile to create thread object for thread 0.");
    }

    core_rtl_array_set(&thread_table, 0, p_thread_obj);

    pcore_sched_info_t p_info = &cpu_infos[0];
    p_info->cpu_use_stat_l = 0;
    p_info->cpu_use_stat_h = 0;
    INC_REF(p_thread_obj);
    p_info->p_idle_thread = p_thread_obj;
    p_info->priority = PRIORITY_DISPATCH;
    p_info->current_node = core_mm_heap_alloc(sizeof(list_node_t), sched_heap);
    p_info->current_node->p_item = p_thread_obj;
    p_thread_obj->p_node = p_info->current_node;
    core_pm_spnlck_init(&(p_info->lock));

    //Initialize schedule list
    core_rtl_memset(sched_lists, 0, sizeof(sched_lists));
    core_rtl_memset(cpu_infos, 0, sizeof(cpu_infos));
    core_pm_spnlck_init(&sched_list_lock);
    core_pm_spnlck_init(&sched_lock);

    //Set schedule callback
    hal_io_int_callback_set(INT_TICK, on_tick);
    initialized = true;

    hal_cpu_regist_IPI_hndlr(IPI_TYPE_PREEMPT, ipi_preempt);
    return;
}

void core_pm_thread_core_init()
{
    u32 cpu_index = hal_cpu_get_cpu_index();
    pcore_sched_info_t p_info = &cpu_infos[cpu_index];
    p_info->cpu_use_stat_l = 0;
    p_info->cpu_use_stat_h = 0;
    p_info->current_node = NULL;
    hal_rtl_atomic_addl(runing_cpu_num, 1);

    //TODO://Create idle thread
    NOT_SUPPORT;
    core_pm_spnlck_init(&(p_info->lock));
    p_info->enabled = true;

    return;
}

void core_pm_thread_core_release()
{
    u32 cpu_index = hal_cpu_get_cpu_index();
    pcore_sched_info_t p_info = &cpu_infos[cpu_index];
    core_pm_spnlck_lock(&(p_info->lock));
    p_info->enabled = false;
    hal_rtl_atomic_subl(runing_cpu_num, 1);

    if(p_info->current_node != NULL) {
        //Get node
        plist_node_t p_node = p_info->current_node;
        p_info->current_node = NULL;
        core_pm_spnlck_unlock(&(p_info->lock));

        //Add Task to schedule list
        pthread_obj_t p_thread_obj = (pthread_obj_t)(p_info->current_node->p_item);

        if(p_thread_obj != p_info->p_idle_thread) {
            core_pm_spnlck_lock(&sched_list_lock);
            pthread_obj_t p_obj = (pthread_obj_t)(p_node->p_item);
            plist_t p_list = &sched_lists[p_obj->priority];

            if(*p_list == NULL) {
                *p_list = p_node;
                p_node->p_prev = p_node;
                p_node->p_next = p_node;

            } else {
                p_node->p_prev = (*p_list)->p_prev;
                p_node->p_next = (*p_list);
                (*p_list)->p_prev->p_next = p_node;
                (*p_list)->p_next->p_prev = p_node;
                *p_list = p_node;
            }
        }

        //TODO:Remove idle thread
        NOT_SUPPORT;

        core_pm_spnlck_unlock(&sched_list_lock);

    } else {
        core_pm_spnlck_unlock(&(p_info->lock));
    }

    return;
}

void core_pm_reg_thread_create_obj(thread_ref_call_back_t callback)
{
    core_pm_spnlck_rw_w_lock(&thread_table_lock);
    //Add callback to list
    core_rtl_list_insert_after(NULL, &thread_ref_callback_list,
                               callback, sched_heap);

    //Add objs to existed threads
    u32 id = 0;

    while(core_rtl_array_get_used_index(&thread_table, id, &id)) {
        add_ref_obj(id, callback);
        id += 1;
    }

    core_pm_spnlck_rw_w_unlock(&thread_table_lock);

    return;
}

u32 core_pm_thread_create(thread_func_t thread_func, u32 k_stack_size,
                          u32 priority, void* p_arg)
{
    u32 process_id = core_pm_get_currnt_proc_id();

    //Allocacate thread id
    core_pm_spnlck_rw_w_lock(&thread_table_lock);

    u32 new_id;

    if(!core_rtl_array_get_free_index(&thread_table, &new_id)) {
        core_pm_spnlck_rw_w_unlock(&thread_table_lock);
        peagain_except_t p_except = eagain_except();
        RAISE(p_except, "Too many threads exists.");
        return 0;
    }

    //Create thread object
    if(k_stack_size == 0) {
        k_stack_size = DEFAULT_STACK_SIZE;
    }

    pthread_obj_t p_thread_obj = thread_obj(new_id, process_id, k_stack_size,
                                            priority);

    if(p_thread_obj == NULL) {
        core_pm_spnlck_rw_w_unlock(&thread_table_lock);
        penomem_except_t p_except = enomem_except();
        RAISE(p_except, "Failed to create thread object.");
        return 0;
    }

    //Add thread to thread table
    core_rtl_array_set(&thread_table, new_id, p_thread_obj);

    //Add referenced objects
    plist_node_t p_node = thread_ref_callback_list;

    if(p_node != NULL) {
        do {
            add_ref_obj(new_id, (thread_ref_call_back_t)(p_node->p_item));
            p_node = p_node->p_next;
        } while(p_node != thread_ref_callback_list);
    }

    core_pm_spnlck_rw_w_unlock(&thread_table_lock);

    //TODO:Add new thread to process

    //Prepare context
    hal_cpu_get_init_kernel_context((void*)(p_thread_obj->k_stack_addr),
                                    p_thread_obj->k_stack_size,
                                    thread_func,
                                    new_id,
                                    p_arg);

    //Resume thread
    core_pm_resume(new_id);

    return new_id;
}

void		core_pm_exit(void* retval)
{
    UNREFERRED_PARAMETER(retval);
}

u32			core_pm_join(bool wait_threadid, u32 thread_id, void** p_retval);

void core_pm_suspend(u32 thread_id)
{
}

void core_pm_resume(u32 thread_id)
{
    //Get thread obj
    core_pm_spnlck_rw_r_lock(&thread_table_lock);
    pthread_obj_t p_thread_obj = core_rtl_array_get(&thread_table, thread_id);

    if(p_thread_obj != NULL) {
        INC_REF(p_thread_obj);
    }

    core_pm_spnlck_rw_r_unlock(&thread_table_lock);

    if(p_thread_obj == NULL) {
        //Raise exception
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal thread id.");
        return;
    }

    //Check thread status
    if(p_thread_obj->status == TASK_ZOMBIE) {
        //Zombie
        //Raise exception
        DEC_REF(p_thread_obj);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Unable to resume an exited thread.");
        return;

    } else {
        if(p_thread_obj->status == TASK_SLEEP) {
            //Sleeping
            core_pm_spnlck_lock(&sched_list_lock);

            //Wakeup thread
            p_thread_obj->wakeup(p_thread_obj);

            if(p_thread_obj->p_node != NULL) {
                if(sched_lists[p_thread_obj->priority]
                   != p_thread_obj->p_node) {
                    //Move the task to the top of schedule list
                    plist_node_t p_node = p_thread_obj->p_node;

                    p_node->p_prev->p_next = p_node->p_next;
                    p_node->p_next->p_prev = p_node->p_prev;

                    p_node->p_prev = sched_lists[p_thread_obj->priority]->p_prev;
                    p_node->p_next = sched_lists[p_thread_obj->priority];
                    p_node->p_prev->p_next = p_node;
                    p_node->p_next->p_prev = p_node;

                    sched_lists[p_thread_obj->priority] = p_node;
                }
            }

            core_pm_spnlck_unlock(&sched_list_lock);

        } else if(p_thread_obj->status == TASK_SUSPEND) {
            //Suspended
            core_pm_spnlck_lock(&sched_list_lock);
            //Set thread status
            p_thread_obj->status = TASK_READY;
            p_thread_obj->status_info.runing.time_slices = 0;
            p_thread_obj->reset_timeslice(p_thread_obj);

            //Insert task to schedule list
            if(p_thread_obj->p_node != NULL) {
                plist_node_t p_node = p_thread_obj->p_node;

                if(sched_lists[p_thread_obj->priority] == NULL) {
                    sched_lists[p_thread_obj->priority] = p_node;
                    p_node->p_prev = p_node;
                    p_node->p_next = p_node;

                } else {
                    p_node->p_prev = sched_lists[p_thread_obj->priority]->p_prev;
                    p_node->p_next = sched_lists[p_thread_obj->priority];
                    p_node->p_prev->p_next = p_node;
                    p_node->p_next->p_prev = p_node;

                    sched_lists[p_thread_obj->priority] = p_node;
                }
            }

            core_pm_spnlck_unlock(&sched_list_lock);
        }

        //Schedule now
        hal_cpu_send_IPI(-1, IPI_TYPE_PREEMPT, NULL);
    }

    return;
}

u32 core_pm_get_currnt_thread_id()
{
    if(!initialized) {
        return 0;
    }

    //Get thread obj
    u32 cpu_index = hal_cpu_get_cpu_index();
    pcore_sched_info_t p_info = &cpu_infos[cpu_index];

    pthread_obj_t p_thread_obj;

    if(p_info->current_node == NULL) {
        p_thread_obj = p_info->p_idle_thread;

    } else {
        p_thread_obj = (pthread_obj_t)(p_info->current_node->p_item);
    }

    return p_thread_obj->thread_id;
}

static int tmp_priority = PRIORITY_DISPATCH;

u32 core_pm_get_currnt_thrd_priority()
{
    if(!initialized) {
        return tmp_priority;
    }

    //Get thread obj
    u32 cpu_index = hal_cpu_get_cpu_index();
    pcore_sched_info_t p_info = &cpu_infos[cpu_index];

    pthread_obj_t p_thread_obj;

    if(p_info->current_node == NULL) {
        p_thread_obj = p_info->p_idle_thread;

    } else {
        p_thread_obj = (pthread_obj_t)(p_info->current_node->p_item);
    }

    return p_thread_obj->priority;
}

void core_pm_set_currnt_thrd_priority(u32 priority)
{
    if(!initialized) {
        tmp_priority = priority;
        return;
    }

    //Get thread obj
    u32 cpu_index = hal_cpu_get_cpu_index();
    pcore_sched_info_t p_info = &cpu_infos[cpu_index];

    pthread_obj_t p_thread_obj;

    if(p_info->current_node == NULL) {
        p_thread_obj = p_info->p_idle_thread;

    } else {
        p_thread_obj = (pthread_obj_t)(p_info->current_node->p_item);
    }

    //Set priority
    p_thread_obj->priority = priority;
    p_info->priority = priority;

    hal_io_int(INT_TICK);
    return;
}

u32			core_pm_get_thrd_priority(u32 thrd_id);
void		core_pm_set_thrd_priority(u32 thrd_id, u32 priority);

void core_pm_schedule()
{
    //Clear timesclice
    u32 cpu_index = hal_cpu_get_cpu_index();
    pcore_sched_info_t p_info = &cpu_infos[cpu_index];

    if(p_info->priority < PRIORITY_DISPATCH) {
        //Get thread obj
        pthread_obj_t p_thread_obj;

        if(p_info->current_node == NULL) {
            p_thread_obj = p_info->p_idle_thread;

        } else {
            p_thread_obj = (pthread_obj_t)(p_info->current_node->p_item);
        }

        if(p_thread_obj->status == TASK_READY
           || p_thread_obj->status == TASK_RUNNING) {
            p_thread_obj->status_info.runing.time_slices = 0;
        }
    }

    //Schedule

    return;
}

void core_pm_idle()
{
    core_pm_set_currnt_thrd_priority(PRIORITY_IDLE);

    //Wait for release command
    while(true);
}

void on_tick(u32 int_num, pcontext_t p_context, u32 err_code)
{
    //Get schedule info
    u32 cpu_index = hal_cpu_get_cpu_index();
    pcore_sched_info_t p_info = &cpu_infos[cpu_index];

    core_pm_spnlck_raw_lock(&sched_lock);

    if(!p_info->enabled || p_info->priority == PRIORITY_HIGHEST) {
        core_pm_spnlck_raw_unlock(&sched_lock);
        hal_io_irq_send_eoi();
        hal_cpu_context_load(p_context);
    }

    //Save context
    pthread_obj_t p_thread_obj;

    if(p_info->current_node == NULL) {
        p_thread_obj = p_info->p_idle_thread;

    } else {
        p_thread_obj = (pthread_obj_t)(p_info->current_node->p_item);
    }

    p_thread_obj->p_context = p_context;
    p_thread_obj->status = TASK_READY;

    //Switch task
    next_task(p_info);
    core_pm_spnlck_raw_unlock(&sched_lock);
    switch_task(p_info, cpu_index);

    UNREFERRED_PARAMETER(int_num);
    UNREFERRED_PARAMETER(err_code);
    return;
}

void add_use_count(pcore_sched_info_t p_info, bool is_busy)
{
    core_pm_spnlck_raw_lock(&(p_info->lock));
    p_info->cpu_use_stat_h <<= 1;

    if(p_info->cpu_use_stat_l & 0x8000000000000000) {
        p_info->cpu_use_stat_h += 0x01;
    }

    p_info->cpu_use_stat_l <<= 1;

    if(is_busy) {
        p_info->cpu_use_stat_l += 1;
    }

    core_pm_spnlck_raw_unlock(&(p_info->lock));
    return;
}

void next_task(pcore_sched_info_t p_info)
{
    //Get current thread object
    pthread_obj_t  p_thread_obj = (pthread_obj_t)(p_info->current_node->p_item);
    p_thread_obj->status = TASK_READY;

    //Search for next task
    plist_node_t p_node = NULL;

    if(!(p_thread_obj->can_run(p_thread_obj))) {
        //Check if current thread can be preempted
        if(p_thread_obj->priority == PRIORITY_HIGHEST) {
            //Highest priority, resume current thread
            add_use_count(p_info, true);
            return;
        }

        //Realtime tasks
        core_pm_spnlck_raw_lock(&sched_list_lock);

        //Traverse schedule lists
        u32 end_priority = MAX(p_info->priority, PRIORITY_DISPATCH - 1);

        for(u32 pri = PRIORITY_HIGHEST;
            pri > end_priority;
            pri--) {

            if(sched_lists[pri] != NULL) {
                p_node = sched_lists[pri];

                if(p_node->p_prev == p_node) {
                    sched_lists[pri] = NULL;

                } else {
                    p_node->p_prev->p_next = p_node->p_next;
                    p_node->p_next->p_prev = p_node->p_prev;
                    sched_lists[pri] = p_node->p_next;
                }

                break;
            }
        }

        //Normal tasks
        if(p_node == NULL) {
            //Traverse schedule lists
            end_priority = MAX(p_info->priority, PRIORITY_IDLE_TASK);
            core_pm_spnlck_raw_lock(&sched_list_lock);

            for(u32 pri = PRIORITY_DISPATCH - 1;
                pri > end_priority;
                pri--) {
                if(sched_lists[pri] != NULL) {
                    //Traverse schedule list
                    plist_node_t p_begin_node = sched_lists[pri];

                    do {
                        pthread_obj_t p_thread_obj = (pthread_obj_t)(
                                                         sched_lists[pri]->p_item);

                        if(p_thread_obj->can_run(p_thread_obj)) {
                            p_node = sched_lists[pri];

                            if(p_node->p_prev == p_node) {
                                sched_lists[pri] = NULL;

                            } else {
                                p_node->p_prev->p_next = p_node->p_next;
                                p_node->p_next->p_prev = p_node->p_prev;
                                sched_lists[pri] = p_node->p_next;
                            }

                            break;
                        }

                        sched_lists[pri] = sched_lists[pri]->p_next;
                    } while(sched_lists[pri] != p_begin_node);

                    if(p_node != NULL) {
                        break;
                    }
                }
            }

            core_pm_spnlck_raw_unlock(&sched_list_lock);
        }

        if(p_node != NULL) {
            //Change current thread
            plist_node_t p_old_node = p_info->current_node;

            if(sched_lists[p_thread_obj->priority] == NULL) {
                sched_lists[p_thread_obj->priority] = p_old_node;
                p_old_node->p_prev = p_old_node;
                p_old_node->p_next = p_old_node;

            } else {
                p_old_node->p_prev = sched_lists[p_thread_obj->priority]->p_prev;
                p_old_node->p_next = sched_lists[p_thread_obj->priority];
                p_old_node->p_prev->p_next = p_old_node;
                p_old_node->p_next->p_prev = p_old_node;
                sched_lists[p_thread_obj->priority] = p_old_node;
            }

            p_info->current_node = p_node;
        }

        core_pm_spnlck_raw_unlock(&sched_list_lock);

    } else {
        core_pm_spnlck_raw_lock(&sched_list_lock);

        //Add current thread to schedule list
        plist_node_t p_old_node = p_info->current_node;

        if(p_thread_obj->status == TASK_RUNNING
           || p_thread_obj->status == TASK_READY
           || p_thread_obj->status == TASK_SLEEP) {

            if(sched_lists[p_thread_obj->priority] == NULL) {
                sched_lists[p_thread_obj->priority] = p_old_node;
                p_old_node->p_prev = p_old_node;
                p_old_node->p_next = p_old_node;

            } else {
                p_old_node->p_prev = sched_lists[p_thread_obj->priority]->p_prev;
                p_old_node->p_next = sched_lists[p_thread_obj->priority];
                p_old_node->p_prev->p_next = p_old_node;
                p_old_node->p_next->p_prev = p_old_node;
            }
        }

        //Look for a thread to run
        for(int i = 0; i < 2; i++) {

            //Realtime tasks
            for(u32 pri = PRIORITY_HIGHEST;
                pri >= PRIORITY_DISPATCH;
                pri--) {
                if(sched_lists[pri] != NULL) {
                    p_node = sched_lists[pri];

                    if(p_node->p_prev == p_node) {
                        sched_lists[pri] = NULL;

                    } else {
                        p_node->p_prev->p_next = p_node->p_next;
                        p_node->p_next->p_prev = p_node->p_prev;
                        sched_lists[pri] = p_node->p_next;
                    }

                    break;
                }
            }

            if(p_node != NULL) {
                break;
            }

            //Normal tasks
            for(u32 pri = PRIORITY_DISPATCH;
                pri >= PRIORITY_IDLE_TASK + 1;
                pri--) {
                if(sched_lists[pri] != NULL) {
                    //Traverse schedule list
                    plist_node_t p_begin_node = sched_lists[pri];

                    do {
                        pthread_obj_t p_thread_obj = (pthread_obj_t)(
                                                         sched_lists[pri]->p_item);

                        if(p_thread_obj->can_run(p_thread_obj)) {
                            p_node = sched_lists[pri];

                            if(p_node->p_prev == p_node) {
                                sched_lists[pri] = NULL;

                            } else {
                                p_node->p_prev->p_next = p_node->p_next;
                                p_node->p_next->p_prev = p_node->p_prev;
                                sched_lists[pri] = p_node->p_next;
                            }

                            break;
                        }

                        sched_lists[pri] = sched_lists[pri]->p_next;
                    } while(sched_lists[pri] != p_begin_node);

                    if(p_node != NULL) {
                        break;
                    }
                }
            }

            if(p_node != NULL) {
                break;

            } else if(i == 0) {
                reset_timeslices();
            }
        }

        //IDLE tasks
        if(sched_lists[PRIORITY_IDLE_TASK] != NULL) {
            for(int i = 0; i < 2; i++) {
                plist_node_t p_begin_node = sched_lists[PRIORITY_IDLE_TASK];

                do {
                    pthread_obj_t p_thread_obj = (pthread_obj_t)(
                                                     sched_lists[PRIORITY_IDLE_TASK]->p_item);

                    if(p_thread_obj->can_run(p_thread_obj)) {
                        p_node = sched_lists[PRIORITY_IDLE_TASK];

                        if(p_node->p_prev == p_node) {
                            sched_lists[PRIORITY_IDLE_TASK] = NULL;

                        } else {
                            p_node->p_prev->p_next = p_node->p_next;
                            p_node->p_next->p_prev = p_node->p_prev;
                            sched_lists[PRIORITY_IDLE_TASK] = p_node->p_next;
                        }

                        break;
                    }

                    sched_lists[PRIORITY_IDLE_TASK] = sched_lists[PRIORITY_IDLE_TASK]->p_next;
                } while(sched_lists[PRIORITY_IDLE_TASK] != p_begin_node);

                if(p_node == NULL && i < 1) {
                    reset_idle_timeslices();

                } else if(p_node != NULL) {
                    break;
                }
            }
        }

        if(p_node == NULL) {
            p_info->current_node = p_info->p_idle_thread->p_node;

        } else {
            p_info->current_node = p_node;
        }

        core_pm_spnlck_raw_unlock(&sched_list_lock);
    }

    return;
}

void switch_task(pcore_sched_info_t p_info, u32 cpu_index)
{
    pthread_obj_t p_thread_obj;

    if(p_info->current_node == NULL) {
        p_thread_obj = p_info->p_idle_thread;
        p_thread_obj->reset_timeslice(p_thread_obj);
        add_use_count(p_info, false);

    } else {
        p_thread_obj = (pthread_obj_t)(p_info->current_node->p_item);
        add_use_count(p_info, true);
    }

    p_info->priority = p_thread_obj->priority;
    p_thread_obj->resume(p_thread_obj, cpu_index);

    return;
}

void reset_timeslices()
{
    for(u32 pri = PRIORITY_DISPATCH - 1;
        pri > PRIORITY_IDLE_TASK;
        pri--) {

        //Traverse schedule list
        plist_node_t p_node = sched_lists[pri];

        if(p_node != NULL) {
            do {
                pthread_obj_t p_thread_obj = (pthread_obj_t)(
                                                 p_node->p_item);
                p_thread_obj->reset_timeslice(p_thread_obj);
                p_node = p_node->p_next;
            } while(sched_lists[pri] != p_node);
        }
    }

    return;
}

void reset_idle_timeslices()
{
    //Traverse schedule list
    plist_node_t p_node = sched_lists[PRIORITY_IDLE_TASK];

    if(p_node != NULL) {
        do {
            pthread_obj_t p_thread_obj = (pthread_obj_t)(
                                             p_node->p_item);
            p_thread_obj->reset_timeslice(p_thread_obj);
            p_node = p_node->p_next;
        } while(sched_lists[PRIORITY_IDLE_TASK] != p_node);
    }

    return;
}

void add_ref_obj(u32 thread_id, thread_ref_call_back_t callback)
{
    //Get thread object
    pthread_obj_t p_thread_obj = core_rtl_array_get(&thread_table, thread_id);

    //Get reference object
    pthread_ref_obj_t p_ref = callback(thread_id);

    //Add reference object
    p_thread_obj->add_ref(p_thread_obj, p_ref);

    return;
}

void ipi_preempt(pcontext_t p_context, pipi_arg_obj_t p_null)
{
    hal_io_int(INT_TICK);
    UNREFERRED_PARAMETER(p_context);
    UNREFERRED_PARAMETER(p_null);
}
