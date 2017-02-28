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
static	void	add_ref_obj(pprocess_obj_t p_this, pproc_ref_obj_t p_ref_obj);
static	void	die(pprocess_obj_t p_this);
static	void	add_child(pprocess_obj_t p_this, u32 child_id);
static	void	zombie_child(pprocess_obj_t p_this, u32 child_id);
static	void	remove_child(pprocess_obj_t p_this, u32 child_id);
static	void	add_thread(pprocess_obj_t p_this, u32 thread_id);
static	void	zombie_thread(pprocess_obj_t p_this, u32 thread_id);
static	void	remove_thread(pprocess_obj_t p_this, u32 thread_id);
static	bool	wait_for_zombie_thread(pprocess_obj_t p_this, bool by_id,
                                       u32* p_thread_id);
static	bool	wait_for_zombie_child(pprocess_obj_t p_this, bool by_id,
                                      u32* p_zombie_child_id);

//Others
static	int				compare_num(u32* p_n1, u32* p_n2);
static	int				compare_addr(address_t addr1, address_t addr2);

pprocess_obj_t process_obj_0()
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
    p_ret->parent_id = 0;
    p_ret->exit_code = 0;
    core_pm_spnlck_init(&(p_ret->lock));

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
    core_pm_event_init(&(p_ret->thrd_wait_event));

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
    core_pm_event_init(&(p_ret->child_wait_event));

    //Methods
    p_ret->fork = fork;
    p_ret->add_ref_obj = add_ref_obj;
    p_ret->die = die;
    p_ret->add_child = add_child;
    p_ret->zombie_child = zombie_child;
    p_ret->remove_child = remove_child;
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
    p_ret->parent_id = p_this->process_id;
    p_ret->exit_code = 0;
    core_pm_spnlck_init(&(p_ret->lock));

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
    core_pm_event_init(&(p_ret->thrd_wait_event));

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
    core_pm_event_init(&(p_ret->child_wait_event));

    //Methods
    p_ret->fork = p_this->fork;
    p_ret->add_ref_obj = p_this->add_ref_obj;
    p_ret->die = p_this->die;
    p_ret->add_child = p_this->add_child;
    p_ret->zombie_child = p_this->zombie_child;
    p_ret->remove_child = p_this->remove_child;
    p_ret->add_thread = p_this->add_thread;
    p_ret->zombie_thread = p_this->zombie_thread;
    p_ret->remove_thread = p_this->remove_thread;
    p_ret->wait_for_zombie_thread = p_this->wait_for_zombie_thread;
    p_ret->wait_for_zombie_child = p_this->wait_for_zombie_child;

    //Fork page table
    core_mm_pg_tbl_fork(p_this->process_id, new_process_id);

    return p_ret;
}

void add_ref_obj(pprocess_obj_t p_this, pproc_ref_obj_t p_ref_obj);
void die(pprocess_obj_t p_this);
void add_child(pprocess_obj_t p_this, u32 child_id);
void zombie_child(pprocess_obj_t p_this, u32 child_id);
void remove_child(pprocess_obj_t p_this, u32 child_id);
void add_thread(pprocess_obj_t p_this, u32 thread_id);
void zombie_thread(pprocess_obj_t p_this, u32 thread_id);
void remove_thread(pprocess_obj_t p_this, u32 thread_id);
bool wait_for_zombie_thread(pprocess_obj_t p_this, bool by_id,
                            u32* p_thread_id);
bool wait_for_zombie_child(pprocess_obj_t p_this, bool by_id,
                           u32* p_zombie_child_id);

int compare_num(u32* p_n1, u32* p_n2)
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
