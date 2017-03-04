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
    core_kconsole_print_info("\nInitializing process table...\n");
    core_rtl_array_init(&process_tbl, MAX_PROCESS_NUM, proc_tbl_heap);
    core_pm_mutex_init(&process_tbl_lck);

    //Create process 0
    core_kconsole_print_info("\nCreating process 0...\n");
    pprocess_obj_t p_proc_0 = process_obj_0(&process_tbl_lck);
    p_proc_0->add_thread(p_proc_0, 0);

    //Add to process table
    core_rtl_array_set(&process_tbl, 0, p_proc_0);

    return;
}

void		core_pm_reg_proc_ref_obj(proc_ref_call_back_t callback);
u32			core_pm_fork(void* child_start_address);
u32			core_pm_wait(bool wait_pid, u32 process_id);
u32			core_pm_get_subsys(u32 pid);
kstatus_t	core_pm_set_subsys(u32 pid, u32 subsys_id);
u32			core_pm_get_uid(u32 pid);
u32			core_pm_get_gid(u32 pid);
u32			core_pm_get_euid(u32 pid);
kstatus_t	core_pm_set_euid(u32 pid, u32 euid);
u32			core_pm_get_egid(u32 pid);
kstatus_t	core_pm_set_egid(u32 pid, u32 egid);
void		core_pm_set_groups(u32* groupids, size_t size);
size_t		core_pm_get_groups(u32* buf, size_t buf_size);
