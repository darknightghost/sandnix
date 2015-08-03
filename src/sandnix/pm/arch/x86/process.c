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


static	u32			get_free_proc_id();
static	u32			fork_descrpitor_list(u32 dest, u32 src);
static	void		descriptor_destroy_callback(void* descriptor);

void init_process()
{
	dbg_print("\nInitializing process...\n");

	//Process table
	rtl_memset(process_table, 0, sizeof(process_table));

	//Lock
	pm_init_spn_lock(&process_table_lock);

	//Heap
	process_heap = mm_hp_create(TASK_QUEUE_HEAP_SIZE, HEAP_MULTITHREAD);

	if(process_heap == NULL) {
		excpt_panic(EFAULT,
		            "Unable to create process heap!\n");
	}

	//Process 0
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

s32	pm_fork()
{
	u32 new_id;
	u32 new_pdt;
	s32 new_thread;

	pm_acqr_spn_lock(&process_table_lock);

	//Allocate process id
	new_id = get_free_proc_id();

	if(new_id == 0) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(EAGAIN);
		return -1;
	}

	//Fork page table
	new_pdt = mm_pg_tbl_fork(process_table[current_process].pdt_id);

	if(new_pdt == 0) {
		process_table[new_id].alloc_flag = false;
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ENOMEM);
		return -1;
	}

	//Fork file descrpitors
	if(fork_descrpitor_list(new_id, current_process) != ESUCCESS) {
		process_table[new_id].alloc_flag = false;
		mm_pg_tbl_free(new_pdt);
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ENOMEM);
		return -1;
	}

	//Fork thread
	new_thread = fork_thread();

	if(new_thread < 0) {
		rtl_list_destroy(&(process_table[new_id].file_desc_list),
		                 process_heap,
		                 descriptor_destroy_callback);
		process_table[new_id].alloc_flag = false;
		mm_pg_tbl_free(new_pdt);
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(EAGAIN);
		return -1;

	} else if(new_thread == 0) {
		return 0;

	} else {
		rtl_list_insert_after(&(process_table[current_process].child_list),
		                      NULL,
		                      (void*)new_id,
		                      process_heap);
		rtl_list_insert_after(&(process_table[new_id].thread_list),
		                      NULL,
		                      (void*)new_thread,
		                      process_heap);
	}

	pm_rls_spn_lock(&process_table_lock);
	return new_id;
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

u32			pm_get_proc_uid(u32 process_id);
bool		pm_set_proc_uid(u32 process_id, u32 uid);

bool		pm_set_proc_euid(u32 process_id, u32 euid);
u32			pm_get_proc_euid(u32 process_id);
u32			pm_add_proc_file_descriptor(u32 process_id, u32 descriptor);
u32			pm_remove_proc_file_descriptor(u32 process_id, u32 descriptor);

void add_proc_thrd(u32 thrd_id, u32 proc_id)
{
	pm_acqr_spn_lock(&process_table_lock);

	if(process_table[proc_id].alloc_flag == false) {
		excpt_panic(ESRCH,
		            "Unavailable process id!\n");
	}

	pm_rls_spn_lock(&process_table_lock);

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

u32 get_free_proc_id()
{
	u32 id;

	for(id = 0; id < MAX_PROCESS_NUM; id++) {
		if(process_table[id].alloc_flag == false) {
			//Initialize process info
			rtl_memset(&process_table[id], 0, sizeof(process_table[id]));
			process_table[id].alloc_flag = true;
			process_table[id].process_name = mm_hp_alloc(
			                                     rtl_strlen(process_table[current_process].process_name) + 1,
			                                     process_heap);
			process_table[id].parent_id = current_process;
			process_table[id].status = PROC_ALIVE;
			process_table[id].priority = process_table[current_process].priority;
			process_table[id].uid = process_table[current_process].uid;
			process_table[id].euid = process_table[current_process].euid;
			return id;
		}
	}

	return 0;
}

u32 fork_descrpitor_list(u32 dest, u32 src)
{
	//TODO:
	return 0;
}

void descriptor_destroy_callback(void* descriptor)
{
	//TODO:
}
