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

#include "./process_obj.h"
#include "../../rtl/rtl.h"
#include "../../mm/mm.h"
#include "../../exception/exception.h"

#include "../pm.h"

#define	MODULE_NAME		core_pm

//Heap
static	pheap_t				proc_obj_heap = NULL;

//Obj methods
static	pkstring_obj_t		to_string(pprocess_obj_t p_this);
static	int					compare(pprocess_obj_t p_this, pprocess_obj_t p_obj2);
static	void				destructor(pprocess_obj_t p_this);

//Process object methods
static	pprocess_obj_t		fork(pprocess_obj_t p_this, u32 new_process_id);
static	void		add_ref_obj(pprocess_obj_t p_this, pproc_ref_obj_t p_ref_obj);
static	void		die(pprocess_obj_t p_this);
static	void		add_thread(pprocess_obj_t p_this, u32 thread_id);
static	void		zombie_thread(pprocess_obj_t p_this, u32 thread_id);
static	void		remove_thread(pprocess_obj_t p_this, u32 thread_id);
static	kstatus_t	wait_for_zombie_thread(pprocess_obj_t p_this, bool by_id,
        u32* p_thread_id);
static	kstatus_t	wait_for_zombie_child(pprocess_obj_t p_this, bool by_id,
        u32* p_zombie_child_id, pprocess_obj_t* p_p_proc_obj);
//Private method
static	void	add_child(pprocess_obj_t p_this, pprocess_obj_t p_child);
static	void	zombie_child(pprocess_obj_t p_this, u32 child_id);
static	void	remove_child(pprocess_obj_t p_this, u32 child_id);

//Others
static	int				compare_num(u32* p_n1, u32* p_n2);
static	int				compare_addr(address_t addr1, address_t addr2);

pprocess_obj_t process_obj_0(pmutex_t p_tbl_lck)
{
    //Initialize heap
    proc_obj_heap = core_mm_heap_create(HEAP_MULITHREAD, 4096);

    if(proc_obj_heap == NULL) {
        penomem_except_t p_except = enomem_except();
        RAISE(p_except, "Failed to create heap for process objects.");
        return NULL;
    }

    //Create process object 0
    pprocess_obj_t p_ret = (pprocess_obj_t)obj(
                               CLASS_PROCESS_OBJ,
                               (destructor_t)destructor,
                               (compare_obj_t)compare,
                               (to_string_t)to_string,
                               proc_obj_heap,
                               sizeof(process_obj_t));

    if(p_ret == NULL) {
        penomem_except_t p_except = enomem_except();
        RAISE(p_except, "Failed to create process object.");
        return NULL;
    }

    //Member variables
    p_ret->process_id = 0;
    p_ret->p_parent = NULL;
    p_ret->status = PROCESS_ALIVE;
    p_ret->exit_code = 0;
    p_ret->cmd_line = kstring("kernel", proc_obj_heap);
    p_ret->p_tbl_lock = p_tbl_lck;

    //Authority
    p_ret->ruid = 0;
    p_ret->euid = 0;
    p_ret->suid = 0;
    p_ret->rgid = 0;
    p_ret->egid = 0;
    p_ret->sgid = 0;
    core_rtl_list_init(&(p_ret->groups));

    //Environment
    core_rtl_map_init(&(p_ret->env_vars),
                      (item_compare_t)core_rtl_strcmp,
                      proc_obj_heap);

    //Threads
    p_ret->alive_thread_num = 0;
    core_rtl_map_init(&(p_ret->alive_threads),
                      (item_compare_t)compare_num,
                      proc_obj_heap);
    core_rtl_map_init(&(p_ret->zombie_threads),
                      (item_compare_t)compare_num,
                      proc_obj_heap);
    core_pm_cond_init(&(p_ret->thrd_wait_cond), p_ret->p_tbl_lock);
    core_rtl_list_init(&p_ret->thrd_wait_list);

    //Referenced objects
    core_rtl_map_init(&(p_ret->ref_objs),
                      (item_compare_t)compare_addr,
                      proc_obj_heap);

    //Child processes
    core_rtl_map_init(&(p_ret->alive_children),
                      (item_compare_t)compare_num,
                      proc_obj_heap);
    core_rtl_map_init(&(p_ret->zombie_children),
                      (item_compare_t)compare_num,
                      proc_obj_heap);
    core_pm_cond_init(&(p_ret->child_wait_cond), p_ret->p_tbl_lock);
    core_rtl_list_init(&p_ret->child_wait_list);

    //Methods
    p_ret->fork = fork;
    p_ret->add_ref_obj = add_ref_obj;
    p_ret->die = die;
    p_ret->add_thread = add_thread;
    p_ret->zombie_thread = zombie_thread;
    p_ret->remove_thread = remove_thread;
    p_ret->wait_for_zombie_thread = wait_for_zombie_thread;
    p_ret->wait_for_zombie_child = wait_for_zombie_child;

    return p_ret;
}

pkstring_obj_t to_string(pprocess_obj_t p_this)
{
    return kstring_fmt("Process object, ID = %u.", p_this->obj.heap,
                       p_this->process_id);
}

int compare(pprocess_obj_t p_this, pprocess_obj_t p_obj2)
{
    if(p_this->process_id > p_obj2->process_id) {
        return 1;

    } else if(p_this->process_id == p_obj2->process_id) {
        return 0;

    } else {
        return -1;
    }
}

void destructor(pprocess_obj_t p_this)
{
    remove_child(p_this->p_parent, p_this->process_id);
    PRIVATE(release_proc_id)(p_this->process_id);
    core_mm_heap_free(p_this, p_this->obj.heap);

    return;
}

pprocess_obj_t fork(pprocess_obj_t p_this, u32 new_process_id)
{
    //Create new object
    pprocess_obj_t p_ret = (pprocess_obj_t)obj(
                               CLASS_PROCESS_OBJ,
                               (destructor_t)destructor,
                               (compare_obj_t)compare,
                               (to_string_t)to_string,
                               proc_obj_heap,
                               sizeof(process_obj_t));

    //Fork variables
    //Member variables
    p_ret->process_id = new_process_id;
    p_ret->p_parent = p_this;
    p_ret->status = PROCESS_ALIVE;
    p_ret->exit_code = 0;
    p_ret->p_tbl_lock = p_this->p_tbl_lock;

    //Authority
    p_ret->ruid = p_this->ruid;
    p_ret->euid = p_this->euid;
    p_ret->suid = p_this->suid;
    p_ret->rgid = p_this->rgid;
    p_ret->egid = p_this->egid;
    p_ret->sgid = p_this->sgid;
    core_rtl_list_init(&(p_ret->groups));

    //Environment
    core_rtl_map_init(&(p_ret->env_vars),
                      (item_compare_t)core_rtl_strcmp,
                      proc_obj_heap);

    //Threads
    p_ret->alive_thread_num = 0;
    core_rtl_map_init(&(p_ret->alive_threads),
                      (item_compare_t)compare_num,
                      proc_obj_heap);
    core_rtl_map_init(&(p_ret->zombie_threads),
                      (item_compare_t)compare_num,
                      proc_obj_heap);
    core_pm_cond_init(&(p_ret->thrd_wait_cond), p_this->p_tbl_lock);
    core_rtl_list_init(&p_ret->thrd_wait_list);

    //Referenced objects
    core_rtl_map_init(&(p_ret->ref_objs),
                      (item_compare_t)compare_addr,
                      proc_obj_heap);

    //Child processes
    core_rtl_map_init(&(p_ret->alive_children),
                      (item_compare_t)compare_num,
                      proc_obj_heap);
    core_rtl_map_init(&(p_ret->zombie_children),
                      (item_compare_t)compare_num,
                      proc_obj_heap);
    core_pm_cond_init(&(p_ret->child_wait_cond), p_this->p_tbl_lock);
    core_rtl_list_init(&p_ret->child_wait_list);

    //Methods
    p_ret->fork = p_this->fork;
    p_ret->add_ref_obj = p_this->add_ref_obj;
    p_ret->die = p_this->die;
    p_ret->add_thread = p_this->add_thread;
    p_ret->zombie_thread = p_this->zombie_thread;
    p_ret->remove_thread = p_this->remove_thread;
    p_ret->wait_for_zombie_thread = p_this->wait_for_zombie_thread;
    p_ret->wait_for_zombie_child = p_this->wait_for_zombie_child;

    //Fork referenced objects
    for(pproc_ref_obj_t p_ref = (pproc_ref_obj_t)
                                core_rtl_map_next(&(p_this->ref_objs), NULL);
        p_ref != NULL;
        p_ref = (pproc_ref_obj_t)
                core_rtl_map_next(&(p_this->ref_objs), p_ref)) {
        pproc_ref_obj_t p_new_ref = p_ref->fork(p_ref, new_process_id);
        core_rtl_map_set(&(p_ret->ref_objs), p_new_ref, p_new_ref);
    }

    //Fork page table
    core_mm_pg_tbl_fork(p_this->process_id, new_process_id);

    //Add child
    add_child(p_this, p_ret);

    return p_ret;
}

void add_ref_obj(pprocess_obj_t p_this, pproc_ref_obj_t p_ref_obj)
{
    core_rtl_map_set(&(p_this->ref_objs), p_ref_obj, p_ref_obj);

    return;
}

void die(pprocess_obj_t p_this)
{
    //Remove all referenced objects
    for(pproc_ref_obj_t p_ref = (pproc_ref_obj_t)
                                core_rtl_map_next(&(p_this->ref_objs), NULL);
        p_ref != NULL;
        p_ref = (pproc_ref_obj_t)
                core_rtl_map_next(&(p_this->ref_objs), p_ref)) {
        core_rtl_map_set(&(p_this->ref_objs), p_ref, NULL);
        DEC_REF(p_ref);
    }

    pprocess_obj_t p_parent = p_this->p_parent;

    if(p_parent == NULL) {
        //Init process exited, kernel panic
        PANIC(EINITEXITED, "Init process has been exited.");
    }

    //Add all zombie children to parent
    for(u32* p_id = (u32*)core_rtl_map_next(
                        &(p_this->zombie_children),
                        NULL);
        p_id != NULL;
        p_id = (u32*)core_rtl_map_next(
                   &(p_this->zombie_children),
                   p_id)) {
        pproc_child_info_t p_ref = core_rtl_map_get(&(p_this->zombie_children), p_id);
        core_rtl_map_set(&(p_this->zombie_children),
                         &(p_ref->p_process->process_id),
                         NULL);
        core_rtl_map_set(&(p_parent->zombie_children),
                         &(p_ref->p_process->process_id),
                         p_ref);
        p_ref->p_process->p_parent = p_parent;
    }

    //Add all alive children to parent
    for(u32* p_id = (u32*)core_rtl_map_next(
                        &(p_this->alive_children),
                        NULL);
        p_id != NULL;
        p_id = (u32*)core_rtl_map_next(
                   &(p_this->alive_children),
                   p_id)) {
        pproc_child_info_t p_ref = core_rtl_map_get(&(p_this->alive_children), p_id);
        core_rtl_map_set(&(p_this->alive_children),
                         &(p_ref->p_process->process_id),
                         NULL);
        core_rtl_map_set(&(p_parent->alive_children),
                         &(p_ref->p_process->process_id),
                         p_ref);
        p_ref->p_process->p_parent = p_parent;
    }

    //Zombie process
    zombie_child(p_this, p_this->process_id);

    //Remove all threads
    for(u32* p_id = (u32*)core_rtl_map_next(
                        &(p_this->zombie_threads),
                        NULL);
        p_id != NULL;
        p_id = (u32*)core_rtl_map_next(
                   &(p_this->zombie_threads),
                   p_id)) {
        pproc_thrd_info_t p_ref = core_rtl_map_get(&(p_this->zombie_threads),
                                  p_id);
        core_rtl_map_set(&(p_parent->zombie_threads),
                         &(p_ref->id),
                         NULL);
        PRIVATE(unref_thread)(p_ref->id);
        core_mm_heap_free(p_ref, p_this->obj.heap);
    }

    //Release page table
    core_mm_pg_tbl_release(p_this->process_id);

    return;
}

void add_child(pprocess_obj_t p_this, pprocess_obj_t p_child)
{
    //Allocate memory
    pproc_child_info_t p_ref = NULL;

    while(p_ref == NULL) {
        p_ref = core_mm_heap_alloc(sizeof(proc_child_info_t),
                                   p_this->obj.heap);
    }

    //Add thread
    p_ref->p_process = p_child;
    p_ref->waited = false;
    p_ref->ref = 0;
    core_pm_cond_init(&(p_ref->cond), p_this->p_tbl_lock);
    core_rtl_map_set(&(p_this->alive_children),
                     &(p_ref->p_process->process_id),
                     p_ref);

    return;
}

void zombie_child(pprocess_obj_t p_this, u32 child_id)
{
    //Get child process
    pproc_child_info_t p_ref = (pproc_child_info_t)core_rtl_map_get(
                                   &(p_this->alive_children),
                                   &child_id);

    if(p_ref == NULL) {
        PANIC(EINVAL, "process %u is not an alive child process of %u.",
              child_id,
              p_this->process_id);
    }

    //Set status to zombie
    p_ref->p_process->status = PROCESS_ZOMBIE;
    core_rtl_map_set(&(p_this->alive_children),
                     &(p_ref->p_process->process_id),
                     NULL);
    core_rtl_map_set(&(p_this->zombie_children),
                     &(p_ref->p_process->process_id),
                     p_ref);

    //Awake wating threads
    if(p_ref->waited) {
        core_pm_cond_signal(&(p_ref->cond), true);

    }

    plist_node_t p_node = p_this->child_wait_list;

    if(p_node != NULL) {
        do {
            INC_REF(p_this);
            pproc_wait_info_t p_info = (pproc_wait_info_t)(p_node->p_item);
            p_info->p_proc_obj = p_this;
            p_node = p_node->p_next;
        } while(p_node != p_this->child_wait_list);
    }

    if(p_ref->waited) {
        DEC_REF(p_this);

    } else {
        if(p_node != NULL) {
            DEC_REF(p_this);
        }
    }

    core_pm_cond_signal(&(p_this->child_wait_cond), true);

    return;
}

void remove_child(pprocess_obj_t p_this, u32 child_id)
{
    core_rtl_map_set(
        &(p_this->zombie_children),
        &child_id,
        NULL);

    return;
}

void add_thread(pprocess_obj_t p_this, u32 thread_id)
{
    //Allocate memory
    pproc_thrd_info_t p_ref = NULL;

    while(p_ref == NULL) {
        p_ref = core_mm_heap_alloc(sizeof(proc_thrd_info_t), p_this->obj.heap);
    }

    //Add thread
    p_ref->id = thread_id;
    core_pm_cond_init(&(p_ref->cond), p_this->p_tbl_lock);
    p_ref->waited = false;
    p_ref->ref = 0;

    proc_obj_heap = NULL;
    core_rtl_map_set(&(p_this->alive_threads), &(p_ref->id), p_ref);

    (p_this->alive_thread_num)++;

    return;
}

void zombie_thread(pprocess_obj_t p_this, u32 thread_id)
{
    //Get thread info
    pproc_thrd_info_t p_ref = core_rtl_map_get(&(p_this->alive_threads),
                              &thread_id);

    if(p_ref == NULL) {
        PANIC(EINVAL, "Thread %u is not an alive child process of %u.",
              thread_id,
              p_this->process_id);
    }


    //Move to zombie list
    core_rtl_map_set(&(p_this->alive_threads),
                     &thread_id,
                     NULL);
    core_rtl_map_set(&(p_this->zombie_threads),
                     &thread_id,
                     p_ref);

    //Awake waiting thread
    if(p_ref->waited) {
        core_pm_cond_signal(&(p_ref->cond), true);

    }

    plist_node_t p_node = p_this->thrd_wait_list;

    if(p_node != NULL) {
        do {
            PRIVATE(ref_thread)(thread_id);
            pthrd_wait_info_t p_info = (pthrd_wait_info_t)(p_node->p_item);
            p_info->id = thread_id;
            p_node = p_node->p_next;
        } while(p_node != p_this->thrd_wait_list);
    }

    core_pm_cond_signal(&(p_this->thrd_wait_cond), true);

    if(p_ref->waited) {
        //Thread has been waited by id when it is alive
        PRIVATE(unref_thread)(thread_id);

    } else if(!p_ref->waited) {
        if(p_node != NULL) {
            //Thread has been waited without id when it is alive
            PRIVATE(unref_thread)(thread_id);
            p_this->remove_thread(p_this, p_ref->id);
        }

        //Thread has not been waited, do nothing
    }

    (p_this->alive_thread_num)--;

    if(p_this->alive_thread_num == 0) {
        p_this->die(p_this);
    }

    return;
}

void remove_thread(pprocess_obj_t p_this, u32 thread_id)
{
    //Get thread info
    pproc_thrd_info_t p_ref = core_rtl_map_get(&(p_this->zombie_threads),
                              &thread_id);

    if(p_ref == NULL) {
        PANIC(EINVAL, "Thread %u is not an zombie child process of %u.",
              thread_id,
              p_this->process_id);
    }

    //Remove thread
    core_rtl_map_set(&(p_this->zombie_threads),
                     &thread_id,
                     NULL);

    //Free memory
    core_mm_heap_free(p_ref, p_this->obj.heap);

    return;
}

kstatus_t wait_for_zombie_thread(pprocess_obj_t p_this, bool by_id,
                                 u32* p_thread_id)
{
    kstatus_t status = ESUCCESS;

    if(by_id) {
        //Thread has been waited by id when it is alive
        //Try to get thread info
        pproc_thrd_info_t p_thrd_info = core_rtl_map_get(
                                            &(p_this->alive_threads),
                                            p_thread_id);

        if(p_thrd_info != NULL) {
            //The thread is alive
            p_thrd_info->waited = true;
            (p_thrd_info->ref)++;
            PRIVATE(ref_thread)(p_thrd_info->id);

            //Wait for zombie
            status = core_pm_cond_wait(&(p_thrd_info->cond), -1);

            if(status != ESUCCESS) {
                PRIVATE(unref_thread)(p_thrd_info->id);
                (p_thrd_info->ref)--;

                if(p_thrd_info->ref == 0) {
                    p_thrd_info->waited = false;
                }

                core_exception_set_errno(status);
                return status;
            }

            (p_thrd_info->ref)--;

            if(p_thrd_info->ref == 0) {
                p_this->remove_thread(p_this, p_thrd_info->id);
            }

            core_exception_set_errno(ESUCCESS);
            return ESUCCESS;

        } else {
            //Thread has been waited by id  when it is zombie
            //The thread is zombie
            p_thrd_info = core_rtl_map_get(
                              &(p_this->zombie_threads),
                              p_thread_id);

            if(p_thrd_info == NULL || p_thrd_info->waited) {
                peinval_except_t p_except = einval_except();
                RAISE(p_except, "Illegal thread id");
                return EINVAL;
            }

            p_thrd_info->waited = true;

            p_this->remove_thread(p_this, p_thrd_info->id);

            core_exception_set_errno(ESUCCESS);
            return ESUCCESS;

        }

    } else {
        //Test if there is any zombie thread
        pproc_thrd_info_t p_thrd_info = NULL;

        for(u32* p_id =
                (u32*)core_rtl_map_next(
                    &(p_this->zombie_threads), NULL);
            p_id != NULL;
            p_id = (u32*)core_rtl_map_next(
                       &(p_this->zombie_threads), p_id)) {
            p_thrd_info = (pproc_thrd_info_t)core_rtl_map_get(
                              &(p_this->zombie_threads),
                              p_id);

            if(!p_thrd_info->waited) {
                break;

            } else {
                p_thrd_info = NULL;
            }
        }

        if(p_thrd_info == NULL) {
            if(p_this->alive_thread_num <= 1) {
                core_exception_set_errno(EDEADLOCK);
                return EDEADLOCK;
            }

            //Thread has been waited without id when it is alive
            //Wait for zombie id
            thrd_wait_info_t wait_info = {
                .id = 0
            };
            list_node_t node = {
                .p_item = &wait_info
            };
            core_rtl_list_insert_node_after(NULL, &(p_this->thrd_wait_list),
                                            &node);
            status = core_pm_cond_wait(&(p_this->thrd_wait_cond),
                                       -1);

            if(status != ESUCCESS) {
                core_exception_set_errno(status);
                return status;
            }

            *p_thread_id = wait_info.id;
            core_rtl_list_node_remove(&node, &(p_this->thrd_wait_list));

            core_exception_set_errno(ESUCCESS);
            return ESUCCESS;

        } else {
            //Thread has been waited without id when it is zombie
            *p_thread_id = p_thrd_info->id;
            p_this->remove_thread(p_this, p_thrd_info->id);
            core_exception_set_errno(ESUCCESS);
            return ESUCCESS;
        }

    }
}

kstatus_t wait_for_zombie_child(pprocess_obj_t p_this, bool by_id,
                                u32* p_zombie_child_id, pprocess_obj_t* p_p_proc_obj)
{
    kstatus_t status = ESUCCESS;

    if(by_id) {
        pproc_child_info_t p_child_info = core_rtl_map_get(
                                              &(p_this->alive_children),
                                              p_zombie_child_id);

        if(p_child_info != NULL) {
            //The child is alive
            p_child_info->waited = true;
            p_child_info->ref++;
            INC_REF(p_child_info->p_process);

            //Wait for zombie
            status = core_pm_cond_wait(&(p_child_info->cond), -1);

            if(status != ESUCCESS) {
                p_child_info->ref--;
                DEC_REF(p_child_info->p_process);

                if(p_child_info->ref == 0) {
                    p_child_info->waited = false;
                }

                core_exception_set_errno(status);
                return status;
            }

            *p_p_proc_obj = p_child_info->p_process;
            p_child_info->ref--;

            if(p_child_info->ref == 0) {
                remove_child(p_this, *p_zombie_child_id);
                core_mm_heap_free(p_child_info, p_this->obj.heap);
            }

            core_exception_set_errno(ESUCCESS);
            return ESUCCESS;

        } else {
            //The child is zombie
            pproc_child_info_t p_child_info = core_rtl_map_get(
                                                  &(p_this->alive_children),
                                                  p_zombie_child_id);

            if(p_child_info == NULL || p_child_info->waited) {
                peinval_except_t p_except = einval_except();
                RAISE(p_except, "Illegal process id");
                return EINVAL;
            }

            *p_p_proc_obj = p_child_info->p_process;
            p_this->remove_thread(p_this, p_child_info->p_process->process_id);
            core_exception_set_errno(ESUCCESS);
            return ESUCCESS;
            remove_child(p_this, *p_zombie_child_id);
            core_mm_heap_free(p_child_info, p_this->obj.heap);

            core_exception_set_errno(ESUCCESS);
            return ESUCCESS;
        }

    } else {
        //Test if there is any zombie child
        pproc_child_info_t p_child_info = NULL;

        for(u32* p_id =
                (u32*)core_rtl_map_next(
                    &(p_this->zombie_threads), NULL);
            p_id != NULL;
            p_id = (u32*)core_rtl_map_next(
                       &(p_this->zombie_threads), p_id)) {
            p_child_info = (pproc_child_info_t)core_rtl_map_get(
                               &(p_this->zombie_threads),
                               p_id);

            if(!p_child_info->waited) {
                break;

            } else {
                p_child_info = NULL;
            }
        }

        if(p_child_info == NULL) {
            //Child has been waited without id when it is alive
            proc_wait_info_t wait_info = {
                .p_proc_obj = NULL
            };
            list_node_t node = {
                .p_item = &wait_info
            };

            core_rtl_list_insert_node_after(NULL, &(p_this->child_wait_list),
                                            &node);
            status = core_pm_cond_wait(&(p_this->child_wait_cond),
                                       -1);

            if(status != ESUCCESS) {
                core_exception_set_errno(status);
                return status;
            }

            *p_p_proc_obj = wait_info.p_proc_obj;
            core_rtl_list_node_remove(&node, &(p_this->child_wait_list));

            core_exception_set_errno(ESUCCESS);
            return ESUCCESS;

        } else {
            //Child has been waited without id when it is zombie
            *p_p_proc_obj = p_child_info->p_process;
            p_this->remove_thread(p_this, p_child_info->p_process->process_id);
            core_exception_set_errno(ESUCCESS);
            return ESUCCESS;
        }
    }
}

int compare_num(u32 * p_n1, u32 * p_n2)
{
    if(*p_n1 > *p_n2) {
        return 1;

    } else if(*p_n1 == *p_n2) {
        return 0;

    } else {
        return -1;
    }
}

int compare_addr(address_t addr1, address_t addr2)
{
    if(addr1 > addr2) {
        return 1;

    } else if(addr1 == addr2) {
        return 0;

    } else {
        return -1;
    }
}
