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

#include "./thread_obj.h"
#include "./thread_ref_obj.h"
#include "../../mm/mm.h"
#include "../../exception/exception.h"
#include "../../rtl/rtl.h"

#include "../../../hal/init/init.h"

static	pheap_t	thread_obj_heap = NULL;

//Obj methods
static	pkstring_obj_t		to_string(pthread_obj_t p_this);
static	int					compare(pthread_obj_t p_this, pthread_obj_t p_obj2);
static	void				destructor(pthread_obj_t p_this);

//Thread object methods
static	void*			add_ref(pthread_obj_t p_this, pthread_ref_obj_t p_obj);
static	void			remove_ref(pthread_obj_t p_this, pthread_ref_obj_t p_obj);
static	pthread_obj_t	fork(pthread_obj_t p_this, u32 new_thread_id, u32 new_proc_id);
static	void			thread_die(pthread_obj_t p_this);
static	void			set_sleep_time(pthread_obj_t p_this, u64* p_ns);


static	int				compare_addr(address_t p_item1, address_t p_item2);
static	void			dec_ref_count(pthread_ref_obj_t p_item, void* p_arg);

pthread_obj_t thread_obj(u32 thread_id, u32 process_id, size_t kernel_stack_size,
                         size_t usr_stack_size, u32 priority)
{
    if(kernel_stack_size == 0) {
        return NULL;
    }

    kernel_stack_size = ALIGN(kernel_stack_size, SANDNIX_KERNEL_PAGE_SIZE);
    usr_stack_size = ALIGN(usr_stack_size, SANDNIX_KERNEL_PAGE_SIZE);

    if(thread_obj_heap == NULL) {
        //Create heap
        thread_obj_heap = core_mm_heap_create(HEAP_MULITHREAD,
                                              SANDNIX_KERNEL_PAGE_SIZE);

        if(thread_obj_heap == NULL) {
            PANIC(ENOMEM, "Failed to alloc new heap.");
        }
    }

    //Create object
    pthread_obj_t p_ret = (pthread_obj_t)obj(
                              CLASS_THREAD_OBJ, (destructor_t)destructor,
                              (compare_obj_t)compare, (to_string_t)to_string,
                              thread_obj_heap, sizeof(thread_obj_t));

    if(p_ret == NULL) {
        return NULL;
    }

    //Initialize vatiables
    p_ret->thread_id = thread_id;
    p_ret->status = TASK_READY;
    p_ret->process_id = process_id;
    p_ret->priority = priority;
    p_ret->p_context = NULL;
    p_ret->status_info.runing.time_slices = 0;
    core_rtl_map_init(&(p_ret->ref_objects), (item_compare_t)compare_addr,
                      thread_obj_heap);

    //Methods
    p_ret->add_ref = add_ref;
    p_ret->remove_ref = remove_ref;
    p_ret->fork = fork;
    p_ret->die = thread_die;
    p_ret->set_sleep_time = set_sleep_time;

    //Allocate stack
    //Kernel stack
    p_ret->k_stack_size = kernel_stack_size;
    p_ret->k_stack_addr = (address_t)core_mm_pg_alloc(NULL, kernel_stack_size,
                          PAGE_ACCESS_RDWR | PAGE_OPTION_KERNEL);

    if(p_ret->k_stack_addr == (address_t)NULL) {
        core_mm_heap_free(p_ret, thread_obj_heap);
        return NULL;
    }

    //User stack
    p_ret->u_stack_size = usr_stack_size;

    if(usr_stack_size == 0) {
        p_ret->u_stack_addr = (address_t)NULL;

    } else {
        p_ret->u_stack_addr = (address_t)core_mm_pg_alloc(NULL, usr_stack_size,
                              PAGE_ACCESS_RDWR | PAGE_OPTION_KERNEL);

        if(p_ret->u_stack_addr == (address_t)NULL) {
            core_mm_pg_free((void*)(p_ret->k_stack_addr));
            core_mm_heap_free(p_ret, thread_obj_heap);
            return NULL;
        }
    }


    return p_ret;
}

pthread_obj_t thread_obj_0()
{
    if(thread_obj_heap == NULL) {
        //Create heap
        thread_obj_heap = core_mm_heap_create(HEAP_MULITHREAD,
                                              SANDNIX_KERNEL_PAGE_SIZE);

        if(thread_obj_heap == NULL) {
            PANIC(ENOMEM, "Failed to alloc new heap.");
        }
    }

    //Create object
    pthread_obj_t p_ret = (pthread_obj_t)obj(
                              CLASS_THREAD_OBJ, (destructor_t)destructor,
                              (compare_obj_t)compare, (to_string_t)to_string,
                              thread_obj_heap, sizeof(thread_obj_t));

    if(p_ret == NULL) {
        return NULL;
    }

    //Initialize vatiables
    p_ret->thread_id = 0;
    p_ret->status = TASK_READY;
    p_ret->process_id = 0;
    p_ret->priority = PRIORITY_DISPATCH;
    p_ret->p_context = NULL;
    p_ret->status_info.runing.time_slices = 0;
    core_rtl_map_init(&(p_ret->ref_objects), (item_compare_t)compare_addr,
                      thread_obj_heap);

    //Methods
    p_ret->add_ref = add_ref;
    p_ret->remove_ref = remove_ref;
    p_ret->fork = fork;
    p_ret->die = thread_die;
    p_ret->set_sleep_time = set_sleep_time;

    //Kernel stack
    p_ret->k_stack_size = DEFAULT_STACK_SIZE;
    p_ret->k_stack_addr = (address_t)init_stack;

    //User stack
    p_ret->u_stack_addr = (address_t)NULL;
    p_ret->u_stack_size = 0;

    return p_ret;
}

pkstring_obj_t to_string(pthread_obj_t p_this)
{
    char* status;
    peinval_except_t p_except;

    switch(p_this->status) {
        case TASK_RUNNING:
            status = "Running";
            break;

        case TASK_READY:
            status = "Ready";
            break;

        case TASK_SUSPEND:
            status = "Suspend";
            break;

        case TASK_SLEEP:
            status = "Sleep";
            break;

        case TASK_ZOMBIE:
            status = "Zombie";
            break;

        default:
            p_except = einval_except();
            RAISE(p_except, "Unknow thread status.");
            status = "Unknow";
    }

    pkstring_obj_t p_ret = kstring_fmt("Thread object at %p, "
                                       "Thread id = %d, "
                                       "Process id = %d, "
                                       "Priority = %d, "
                                       "Status = %s.",
                                       thread_obj_heap,
                                       p_this,
                                       p_this->thread_id,
                                       p_this->process_id,
                                       p_this->priority,
                                       status);

    return p_ret;
}

int compare(pthread_obj_t p_this, pthread_obj_t p_obj2)
{
    if(p_this->thread_id > p_obj2->thread_id) {
        return 1;

    } else if(p_this->thread_id == p_obj2->thread_id) {
        return 0;

    } else {
        return -1;
    }
}

void destructor(pthread_obj_t p_this)
{
    //Free object
    core_mm_heap_free(p_this, p_this->obj.heap);

    return;
}

void* add_ref(pthread_obj_t p_this, pthread_ref_obj_t p_obj)
{
    return core_rtl_map_set(&(p_this->ref_objects),
                            p_obj,
                            p_obj);
}

void remove_ref(pthread_obj_t p_this, pthread_ref_obj_t p_obj)
{
    core_rtl_map_set(&(p_this->ref_objects),
                     p_obj,
                     NULL);
    DEC_REF(p_obj);
    return;
}

pthread_obj_t fork(pthread_obj_t p_this, u32 new_thread_id, u32 new_proc_id)
{
    pthread_obj_t p_ret = thread_obj(new_thread_id, new_proc_id,
                                     p_this->k_stack_size, 0,
                                     p_this->priority);

    if(p_ret == NULL) {
        return NULL;
    }

    p_ret->u_stack_addr = p_this->u_stack_addr;
    p_ret->u_stack_size = p_this->u_stack_size;

    //Fork objects
    for(pthread_ref_obj_t p_ref = core_rtl_map_next(&(p_this->ref_objects), NULL);
        p_ref != NULL;
        p_ref = core_rtl_map_next(&(p_this->ref_objects), p_ref)) {
        pthread_ref_obj_t p_new_ref = p_ref->fork(p_ref, new_thread_id);
        p_ret->add_ref(p_ret, p_new_ref);
    }

    return p_ret;
}

void thread_die(pthread_obj_t p_this)
{
    //Decrease refererence of all objs
    core_rtl_map_destroy(&(p_this->ref_objects),
                         (item_destroyer_t)dec_ref_count, NULL, NULL);

    //Free stacks
    if(p_this->k_stack_addr != (address_t)init_stack) {
        core_mm_pg_free((void*)(p_this->k_stack_addr));
    }

    if(p_this->u_stack_size != 0) {
        u32 current_page_id = core_mm_get_current_pg_tbl_index();
        core_mm_switch_to(p_this->process_id);
        core_mm_pg_free((void*)p_this->u_stack_addr);
        core_mm_switch_to(current_page_id);
    }

    return;
}

void set_sleep_time(pthread_obj_t p_this, u64* p_ns)
{
    //TODO:
    UNREFERRED_PARAMETER(p_this);
    UNREFERRED_PARAMETER(p_ns);
    return;
}

int compare_addr(address_t p_item1, address_t p_item2)
{
    if(p_item1 > p_item2) {
        return 1;

    } else if(p_item1 == p_item2) {
        return 0;

    } else {
        return -1;
    }
}

void dec_ref_count(pthread_ref_obj_t p_item, void* p_arg)
{
    DEC_REF(p_item);
    UNREFERRED_PARAMETER(p_arg);
    return;
}
