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

#include "./process.h"
#include "./process_obj.h"
#include "../lock/mutex/mutex.h"
#include "../../mm/mm.h"
#include "../../rtl/rtl.h"
#include "../../kconsole/kconsole.h"
#include "../../exception/exception.h"
#include "../../../hal/mmu/mmu.h"

#define	MODULE_NAME		core_pm

static	array_t			process_tbl;
static	mutex_t			process_tbl_lck;
static	pheap_t			proc_tbl_heap;

void PRIVATE(process_init)()
{
    //Initiale heap
    proc_tbl_heap = core_mm_heap_create(HEAP_MULITHREAD,
                                        SANDNIX_KERNEL_PAGE_SIZE);

    if(proc_tbl_heap == NULL) {
        PANIC(ENOMEM, "Failed to create process table heap.");
    }

    //Initialize process table
    core_kconsole_print_info("Initializing process table...\n");
    core_rtl_array_init(&process_tbl, MAX_PROCESS_NUM, proc_tbl_heap);
    core_pm_mutex_init(&process_tbl_lck);

    //Create process 0
    core_kconsole_print_info("Creating process 0...\n");
    pprocess_obj_t p_proc_0 = process_obj_0(&process_tbl_lck);
    p_proc_0->add_thread(p_proc_0, 0);

    //Add to process table
    core_rtl_array_set(&process_tbl, 0, p_proc_0);

    return;
}

void PRIVATE(add_thread)(u32 process_id, u32 thread_id)
{
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    while(status != ESUCCESS) {
        status = core_pm_mutex_acquire(&process_tbl_lck, -1);
    }

    //Get process object
    pprocess_obj_t p_proc_obj = core_rtl_array_get(&process_tbl, process_id);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal process id.");
        return;
    }

    //Check process status
    if(p_proc_obj->status != PROCESS_ALIVE) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Cannot add new thread to a zombie process.");
        return;
    }

    //Add thread
    p_proc_obj->add_thread(p_proc_obj, thread_id);

    core_pm_mutex_release(&process_tbl_lck);
    return;
}

void PRIVATE(zombie_process_thrd)(u32 process_id, u32 thread_id)
{
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    while(status != ESUCCESS) {
        status = core_pm_mutex_acquire(&process_tbl_lck, -1);
    }

    //Get process object
    pprocess_obj_t p_proc_obj = core_rtl_array_get(&process_tbl, process_id);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal process id.");
        return;
    }

    //Check process status
    if(p_proc_obj->status != PROCESS_ALIVE) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Cannot add new thread to a zombie process.");
        return;
    }

    //Zombie thread
    p_proc_obj->zombie_thread(p_proc_obj, thread_id);

    core_pm_mutex_release(&process_tbl_lck);
    return;
}

void PRIVATE(remove_process_thrd)(u32 process_id, u32 thread_id)
{
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    while(status != ESUCCESS) {
        status = core_pm_mutex_acquire(&process_tbl_lck, -1);
    }

    //Get process object
    pprocess_obj_t p_proc_obj = core_rtl_array_get(&process_tbl, process_id);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal process id.");
        return;
    }

    //Check process status
    if(p_proc_obj->status != PROCESS_ALIVE) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Cannot add new thread to a zombie process.");
        return;
    }

    //Remove thread
    p_proc_obj->remove_thread(p_proc_obj, thread_id);

    core_pm_mutex_release(&process_tbl_lck);
    return;
}

void PRIVATE(release_proc_id)(u32 id)
{
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    while(status != ESUCCESS) {
        status = core_pm_mutex_acquire(&process_tbl_lck, -1);
    }

    core_rtl_array_set(&process_tbl, id, NULL);

    core_pm_mutex_release(&process_tbl_lck);
    return;
}

void		core_pm_reg_proc_ref_obj(proc_ref_call_back_t callback);
u32			core_pm_fork(void* child_start_address);
u32			core_pm_wait(bool wait_pid, u32 process_id);

u32 core_pm_get_subsys(u32 pid)
{
    //Get process object
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    if(status != ESUCCESS) {
        core_exception_set_errno(status);
        return 0;
    }

    pprocess_obj_t p_proc_obj = core_rtl_array_get(
                                    &process_tbl,
                                    pid);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Inllegal process id.");
        return 0;
    }

    INC_REF(p_proc_obj);
    core_pm_mutex_release(&process_tbl_lck);

    //Get subsystem id
    u32 ret = p_proc_obj->subsys;

    DEC_REF(p_proc_obj);
    core_exception_set_errno(ESUCCESS);

    return ret;
}

kstatus_t core_pm_set_subsys(u32 pid, u32 subsys_id)
{
    //Get process object
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    if(status != ESUCCESS) {
        core_exception_set_errno(status);
        return status;
    }

    pprocess_obj_t p_proc_obj = core_rtl_array_get(
                                    &process_tbl,
                                    pid);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Inllegal process id.");
        return EINVAL;
    }

    //Set subsystem id
    p_proc_obj->subsys = subsys_id;
    core_pm_mutex_release(&process_tbl_lck);

    core_exception_set_errno(ESUCCESS);

    return ESUCCESS;
}

u32 core_pm_get_uid(u32 pid)
{
    //Get process object
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    if(status != ESUCCESS) {
        core_exception_set_errno(status);
        return 0;
    }

    pprocess_obj_t p_proc_obj = core_rtl_array_get(
                                    &process_tbl,
                                    pid);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Inllegal process id.");
        return 0;
    }

    INC_REF(p_proc_obj);
    core_pm_mutex_release(&process_tbl_lck);

    //Get real uid
    u32 ret = p_proc_obj->ruid;

    DEC_REF(p_proc_obj);
    core_exception_set_errno(ESUCCESS);

    return ret;
}

u32 core_pm_get_gid(u32 pid)
{
    //Get process object
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    if(status != ESUCCESS) {
        core_exception_set_errno(status);
        return 0;
    }

    pprocess_obj_t p_proc_obj = core_rtl_array_get(
                                    &process_tbl,
                                    pid);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Inllegal process id.");
        return 0;
    }

    INC_REF(p_proc_obj);
    core_pm_mutex_release(&process_tbl_lck);

    //Get real gid
    u32 ret = p_proc_obj->rgid;

    DEC_REF(p_proc_obj);
    core_exception_set_errno(ESUCCESS);

    return ret;
}

u32 core_pm_get_euid(u32 pid)
{
    //Get process object
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    if(status != ESUCCESS) {
        core_exception_set_errno(status);
        return 0;
    }

    pprocess_obj_t p_proc_obj = core_rtl_array_get(
                                    &process_tbl,
                                    pid);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Inllegal process id.");
        return 0;
    }

    INC_REF(p_proc_obj);
    core_pm_mutex_release(&process_tbl_lck);

    //Get effective uid
    u32 ret = p_proc_obj->euid;

    DEC_REF(p_proc_obj);
    core_exception_set_errno(ESUCCESS);

    return ret;
}

kstatus_t core_pm_set_euid(u32 pid, u32 euid)
{
    //Get process object
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    if(status != ESUCCESS) {
        core_exception_set_errno(status);
        return status;
    }

    pprocess_obj_t p_proc_obj = core_rtl_array_get(
                                    &process_tbl,
                                    pid);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Inllegal process id.");
        return EINVAL;
    }

    //Set effective uid
    p_proc_obj->euid = euid;

    core_pm_mutex_release(&process_tbl_lck);

    core_exception_set_errno(ESUCCESS);

    return ESUCCESS;
}

u32 core_pm_get_egid(u32 pid)
{
    //Get process object
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    if(status != ESUCCESS) {
        core_exception_set_errno(status);
        return 0;
    }

    pprocess_obj_t p_proc_obj = core_rtl_array_get(
                                    &process_tbl,
                                    pid);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Inllegal process id.");
        return 0;
    }

    INC_REF(p_proc_obj);
    core_pm_mutex_release(&process_tbl_lck);

    //Get effective gid
    u32 ret = p_proc_obj->egid;

    DEC_REF(p_proc_obj);
    core_exception_set_errno(ESUCCESS);

    return ret;
}

kstatus_t core_pm_set_egid(u32 pid, u32 egid)
{
    //Get process object
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    if(status != ESUCCESS) {
        core_exception_set_errno(status);
        return status;
    }

    pprocess_obj_t p_proc_obj = core_rtl_array_get(
                                    &process_tbl,
                                    pid);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Inllegal process id.");
        return EINVAL;
    }

    //Set effective gid
    p_proc_obj->egid = egid;

    core_pm_mutex_release(&process_tbl_lck);

    core_exception_set_errno(ESUCCESS);

    return ESUCCESS;
}

void core_pm_set_groups(u32 pid, u32* groupids, size_t size)
{
    //Get process object
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    if(status != ESUCCESS) {
        core_exception_set_errno(status);
        return;
    }

    pprocess_obj_t p_proc_obj = core_rtl_array_get(
                                    &process_tbl,
                                    pid);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Inllegal process id.");
        return;
    }

    //Clear groups
    core_rtl_list_destroy(
        &(p_proc_obj->groups),
        proc_tbl_heap,
        NULL,
        NULL);

    //Add groups
    for(u32* p_grp = groupids;
        (address_t)p_grp < (address_t)groupids + size;
        p_grp++) {
        core_rtl_list_insert_after(
            NULL,
            &(p_proc_obj->groups),
            (void*)(*p_grp),
            proc_tbl_heap);
    }

    core_pm_mutex_release(&process_tbl_lck);

    core_exception_set_errno(ESUCCESS);

    return;
}

size_t core_pm_get_groups(u32 pid, u32* buf, size_t buf_size)
{
    //Get process object
    kstatus_t status = core_pm_mutex_acquire(&process_tbl_lck, -1);

    if(status != ESUCCESS) {
        core_exception_set_errno(status);
        return 0;
    }

    pprocess_obj_t p_proc_obj = core_rtl_array_get(
                                    &process_tbl,
                                    pid);

    if(p_proc_obj == NULL) {
        core_pm_mutex_release(&process_tbl_lck);
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Inllegal process id.");
        return 0;
    }

    //Get group ids
    size_t ret = 0;
    plist_node_t p_node = p_proc_obj->groups;
    u32* p_grp = buf;

    if(p_node != NULL) {
        do {
            if((address_t)p_grp - (address_t)buf < buf_size) {
                *p_grp = (u32)(p_node->p_item);
            }

            ret += sizeof(u32);
            p_grp++;
            p_node = p_node->p_next;
        } while(p_node != p_proc_obj->groups);
    }

    core_pm_mutex_release(&process_tbl_lck);

    core_exception_set_errno(ESUCCESS);

    return ret;
}
