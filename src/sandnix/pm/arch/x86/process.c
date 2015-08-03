/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "../../pm.h"
#include "process.h"
#include "../../../rtl/rtl.h"
#include "../../../debug/debug.h"
#include "../../../mm/mm.h"
#include "../../../exceptions/exceptions.h"
#include "../../../vfs/vfs.h"

spin_lock		process_table_lock;
process_info	process_table[MAX_PROCESS_NUM];
void*			process_heap;

void init_process()
{
	dbg_print("\nInitializing process...\n");
	rtl_memset(process_table, 0, sizeof(process_table));
	pm_init_spn_lock(&process_table_lock);

	process_heap = mm_hp_create(TASK_QUEUE_HEAP_SIZE, HEAP_MULTITHREAD);

	if(process_heap == NULL) {
		excpt_panic(EFAULT,
		            "Unable to create process heap!\n");
	}

	dbg_print("Creating process 0...\n");
	process_table[0].alloc_flag = true;
	rtl_list_insert_after(
	    &(process_table[0].thread_list),
	    NULL,
	    (void*)0,
	    process_heap);
	process_table[0].process_name = "system";
	return;
}

u32	pm_fork()
{
	//TODO:
	//Allocate process id
	//Fork page table
	//Fork file descrpitors
	//Fork thread
	return 0;
}

void pm_exit(u32 exit_code)
{
	//TODO:
}

void pm_exec(char* cmd_line)
{
	//TODO:
}

u32 pm_switch_process(u32 process_id)
{
	u32 pdt_id;

	if(process_table[process_id].alloc_flag == false) {
		excpt_panic(ESRCH,
		            "Unavailable process id!\n");
	}

	pdt_id = process_table[process_id].pdt_id;
	mm_pg_tbl_switch(pdt_id);
	return 0;
}

u32			pm_get_pdt_id(u32 process_id);

u32 pm_get_proc_id(u32 thread_id)
{
	return 0;
}

u32			pm_get_proc_uid(u32 process_id);
bool		pm_set_proc_uid(u32 process_id, u32 uid);

void add_proc_thrd(u32 thrd_id, u32 proc_id)
{
	pm_acqr_spn_lock(&process_table_lock);

	if(process_table[proc_id].alloc_flag == false) {
		excpt_panic(ESRCH,
		            "Unavailable process id!\n");
	}

	return;
}

void zombie_proc_thrd(u32 thrd_id, u32 proc_id)
{
	return;
}

void remove_proc_thrd(u32 thrd_id, u32 proc_id)
{
	return;
}

