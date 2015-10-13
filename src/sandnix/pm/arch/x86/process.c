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
#include "elf.h"
#include "../../../rtl/rtl.h"
#include "../../../debug/debug.h"
#include "../../../mm/mm.h"
#include "../../../exceptions/exceptions.h"
#include "../../../vfs/vfs.h"

spinlock_t		process_table_lock;
process_info_t	process_table[MAX_PROCESS_NUM];
void*			process_heap;


static	u32			get_free_proc_id();
static	u32			release_process(u32 process_id);
static	void		awake_threads(list_t lst, u32 status);

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
	process_table[0].is_driver = true;
	return;
}

s32	pm_fork()
{
	u32 new_id;
	u32 new_pdt;
	s32 new_thread;

	adjust_int_level();
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

	process_table[new_id].pdt_id = new_pdt;

	//Fork file descrpitors
	if(vfs_fork(new_id) != OPERATE_SUCCESS) {
		process_table[new_id].alloc_flag = false;
		mm_pg_tbl_free(new_pdt);
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ENOMEM);
		return -1;
	}

	//Fork thread
	new_thread = fork_thread(new_id);

	if(new_thread < 0) {
		vfs_clean(new_id);
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

	pm_set_errno(ESUCCESS);
	pm_rls_spn_lock(&process_table_lock);
	pm_resume_thrd(new_thread);
	return new_id;
}

void pm_execve(char* file_name, char* argv[], char* envp[])
{
	size_t name_len;

	//TODO:exec
	if(file_name == NULL
		||argv==NULL
		||envp==NULL) {
		pm_set_errno(EINVAL);
		return;
	}
	
	//Check thread number
	//Check file type
	//Save arguments
	//Clear user memory
	//Load elf file
	//Return to user memory

	UNREFERRED_PARAMETER(envp);
}

u32 pm_wait(u32 child_id, u32* p_id, bool if_block)
{
	plist_node_t p_node;
	u32 ret;

	pm_acqr_spn_lock(&process_table_lock);

	//Look for child
	if(child_id == 0) {
		if(process_table[current_process].child_list == NULL
		   && process_table[current_process].wait_list == NULL) {
			pm_rls_spn_lock(&process_table_lock);
			pm_set_errno(ECHILD);
			return 0;
		}

	} else {
		//Child
		p_node = process_table[current_process].child_list;

		if(p_node != NULL) {
			do {
				if((u32)(p_node->p_item) == child_id) {
					goto _HAS_CHILD;
				}

				p_node = p_node->p_next;
			} while(p_node != process_table[current_process].child_list);
		}

		//Zombie
		p_node = process_table[current_process].wait_list;

		if(p_node != NULL) {
			do {
				if((u32)(p_node->p_item) == child_id) {
					goto _HAS_CHILD;
				}

				p_node = p_node->p_next;
			} while(p_node != process_table[current_process].wait_list);
		}

		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ECHILD);
		return 0;
_HAS_CHILD:
		__asm__ __volatile__("");
	}

	//Wait
	do {
		if(pm_should_break()) {
			pm_rls_spn_lock(&process_table_lock);
			pm_set_errno(EINTR);
			return 0;
		}

		//Check if the thread is zombie
		p_node = process_table[current_process].wait_list;

		if(p_node != NULL) {
			do {
				if(child_id == 0 ||
				   (u32)(p_node->p_item) == child_id) {
					//Return
					*p_id = (u32)(p_node->p_item);
					ret = release_process((u32)(p_node->p_item));
					rtl_list_remove(&(process_table[current_process].wait_list),
					                p_node,
					                process_heap);
					pm_rls_spn_lock(&process_table_lock);
					pm_set_errno(ESUCCESS);
					return ret;
				}

				p_node = p_node->p_next;
			} while(p_node != process_table[current_process].wait_list);
		}

		if(process_table[current_process].child_list == NULL) {
			pm_rls_spn_lock(&process_table_lock);
			pm_set_errno(ECHILD);
			return 0;

		} else if(child_id != 0) {
			//Check if the child process exisis
			p_node = process_table[current_process].child_list;

			if(p_node != NULL) {
				do {
					if((u32)(p_node->p_item) == child_id) {
						goto _CHILD_EXISIS;
					}

					p_node = p_node->p_next;
				} while(p_node != process_table[current_process].child_list);
			}

			pm_rls_spn_lock(&process_table_lock);
			pm_set_errno(ECHILD);
			return 0;
_CHILD_EXISIS:
			__asm__ __volatile__("");
		}

		//Set thread status
		pm_rls_spn_lock(&process_table_lock);

		if((!pm_should_break()) && if_block) {
			wait_thread();
		}

		pm_acqr_spn_lock(&process_table_lock);

	} while(if_block);

	//Clear zombie threads
	pm_rls_spn_lock(&process_table_lock);

	return 0;
}

u32 pm_get_crrnt_process()
{
	return current_process;
}

bool pm_get_proc_uid(u32 process_id, u32* p_uid)
{
	pm_acqr_spn_lock(&process_table_lock);

	//Check if the process exists
	if(!process_table[process_id].alloc_flag) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ESRCH);
		return false;
	}

	*p_uid = process_table[process_id].uid;
	pm_rls_spn_lock(&process_table_lock);

	pm_set_errno(ESUCCESS);

	return true;
}

bool pm_set_proc_euid(u32 process_id, u32 euid)
{
	pm_acqr_spn_lock(&process_table_lock);

	//Check if the process exists
	if(!process_table[process_id].alloc_flag) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ESRCH);
		return false;
	}

	if(euid != process_table[process_id].euid
	   && euid != process_table[process_id].uid
	   && euid != process_table[process_id].suid) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(EPERM);
		return false;
	}

	if(process_table[process_id].euid == 0) {
		process_table[process_id].uid = euid;
	}

	process_table[process_id].euid = euid;

	pm_rls_spn_lock(&process_table_lock);

	pm_set_errno(ESUCCESS);

	return true;
}

bool pm_get_proc_euid(u32 process_id, u32* p_euid)
{
	pm_acqr_spn_lock(&process_table_lock);

	//Check if the process exists
	if(!process_table[process_id].alloc_flag) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ESRCH);
		return false;
	}

	*p_euid = process_table[process_id].euid;
	pm_rls_spn_lock(&process_table_lock);

	pm_set_errno(ESUCCESS);

	return true;
}

bool pm_get_proc_gid(u32 process_id, u32* p_gid)
{
	pm_acqr_spn_lock(&process_table_lock);

	//Check if the process exists
	if(!process_table[process_id].alloc_flag) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ESRCH);
		return false;
	}

	*p_gid = process_table[process_id].gid;
	pm_rls_spn_lock(&process_table_lock);

	pm_set_errno(ESUCCESS);

	return true;
}

bool pm_set_proc_egid(u32 process_id, u32 egid)
{
	pm_acqr_spn_lock(&process_table_lock);

	//Check if the process exists
	if(!process_table[process_id].alloc_flag) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ESRCH);
		return false;
	}

	if(egid != process_table[process_id].egid
	   && egid != process_table[process_id].gid
	   && egid != process_table[process_id].sgid) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(EPERM);
		return false;
	}

	if(process_table[process_id].euid == 0) {
		process_table[process_id].gid = egid;
	}

	process_table[process_id].egid = egid;

	pm_rls_spn_lock(&process_table_lock);

	pm_set_errno(ESUCCESS);

	return true;
}

bool pm_get_proc_egid(u32 process_id, u32* p_egid)
{
	pm_acqr_spn_lock(&process_table_lock);

	//Check if the process exists
	if(!process_table[process_id].alloc_flag) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ESRCH);
		return false;
	}

	*p_egid = process_table[process_id].egid;
	pm_rls_spn_lock(&process_table_lock);

	pm_set_errno(ESUCCESS);

	return true;
}

void pm_change_to_usr_process()
{
	process_table[current_process].is_driver = false;
	pm_set_errno(ESUCCESS);
	return;
}

bool pm_is_driver()
{
	return process_table[current_process].is_driver;
}

void pm_change_name(char* new_name)
{
	//Change the name of process
	name_len = rtl_strlen(new_name) + 1;
	pm_acqr_spn_lock(&process_table_lock);
	mm_hp_free(process_table[current_process].process_name,
			   process_heap);
	process_table[current_process].process_name = mm_hp_alloc(name_len,
			process_heap);
	rtl_strcpy_s(process_table[current_process].process_name,
				 name_len,
				 new_name);
	pm_rls_spn_lock(&process_table_lock);
	pm_set_errno(ESUCCESS);
	return;
}

void add_proc_thrd(u32 thrd_id, u32 proc_id)
{
	if(process_table[proc_id].alloc_flag == false) {
		excpt_panic(ESRCH,
		            "Unavailable process id!\n");
	}

	//Add new thread to thread list_t
	if(rtl_list_insert_after(&(process_table[proc_id].thread_list),
	                         NULL,
	                         (void*)thrd_id,
	                         process_heap) == NULL) {
		excpt_panic(EFAULT,
		            "Failed to add thread!\n");
	}

	return;
}

void zombie_proc_thrd(u32 thrd_id, u32 proc_id)
{
	plist_node_t p_node;

	if(process_table[proc_id].alloc_flag == false) {
		excpt_panic(ESRCH,
		            "Unavailable process id!\n");
	}

	//Remove from thread list_t
	p_node = rtl_list_get_node_by_item(process_table[proc_id].thread_list,
	                                   (void*)thrd_id);

	if(p_node == NULL) {
		excpt_panic(EFAULT,
		            "Failed to zombie thread!\n");
	}

	rtl_list_remove(&(process_table[proc_id].thread_list),
	                p_node,
	                process_heap);

	//Add to zombie list_t
	if(rtl_list_insert_after(&(process_table[proc_id].zombie_list),
	                         NULL,
	                         (void*)thrd_id,
	                         process_heap) == NULL) {
		excpt_panic(EFAULT,
		            "Failed to zombie thread!\n");
	}

	//Check if the process has any alive thread
	if(process_table[proc_id].thread_list == NULL) {

		//Zombie the process
		process_table[proc_id].status = PROC_ZOMBIE;
		process_table[proc_id].exit_code = thread_table[thrd_id].exit_code;

		//Add process to wait list_t
		rtl_list_insert_after(&(process_table[
		                            process_table[proc_id].parent_id].wait_list),
		                      NULL,
		                      (void*)proc_id,
		                      process_heap);

		//Close all file descriptors
		vfs_clean(proc_id);

		//Copy child list_t
		p_node = process_table[proc_id].child_list;

		while(p_node != NULL) {
			if(rtl_list_insert_after(&(process_table[
			                               process_table[proc_id].parent_id].child_list),
			                         NULL,
			                         p_node->p_item,
			                         process_heap) == NULL) {
				excpt_panic(ENOMEM,
				            "Copy child process list_t from process %d to process %d fault!\n",
				            proc_id,
				            process_table[proc_id].parent_id);
			}

			process_table[(u32)(p_node->p_item)].parent_id = process_table[proc_id].parent_id;

			rtl_list_remove(&(process_table[proc_id].child_list),
			                p_node,
			                process_heap);
			p_node = process_table[proc_id].child_list;
		}

		//Copy wait list_t
		p_node = process_table[proc_id].wait_list;

		while(p_node != NULL) {
			if(rtl_list_insert_after(&(process_table[
			                               process_table[proc_id].parent_id].wait_list),
			                         NULL,
			                         p_node->p_item,
			                         process_heap) == NULL) {
				excpt_panic(ENOMEM,
				            "Copy zombie process list_t from process %d to process %d fault!\n",
				            proc_id,
				            process_table[proc_id].parent_id);
			}

			process_table[(u32)(p_node->p_item)].parent_id = process_table[proc_id].parent_id;

			rtl_list_remove(&(process_table[proc_id].wait_list),
			                p_node,
			                process_heap);
			p_node = process_table[proc_id].wait_list;
		}

		//Awake waiting threads of parent process
		awake_threads(process_table[
		                  process_table[proc_id].parent_id].thread_list,
		              TASK_WAIT);

	} else {
		//Awake joining threads
		awake_threads(process_table[proc_id].thread_list,
		              TASK_JOIN);

	}

	return;
}

void switch_process(u32 process_id)
{
	u32 pdt_id;

	if(process_table[process_id].alloc_flag == false) {
		excpt_panic(ESRCH,
		            "Unavailable process id!\n");
	}

	pdt_id = process_table[process_id].pdt_id;
	mm_pg_tbl_switch(pdt_id);
	return;
}

void set_exit_code(u32 process, u32 exit_code)
{
	process_table[process].exit_code = exit_code;
	return;
}

u32 get_process_pdt(u32 process_id)
{
	return process_table[process_id].pdt_id;
}

u32 get_free_proc_id()
{
	u32 id;
	size_t name_len;

	for(id = 0; id < MAX_PROCESS_NUM; id++) {
		if(process_table[id].alloc_flag == false) {
			//Initialize process info
			rtl_memset(&process_table[id], 0, sizeof(process_table[id]));
			process_table[id].alloc_flag = true;
			name_len = rtl_strlen(process_table[current_process].process_name) + 1;
			process_table[id].process_name = mm_hp_alloc(
			                                     name_len,
			                                     process_heap);
			rtl_strcpy_s(process_table[id].process_name,
			             name_len,
			             process_table[current_process].process_name);
			process_table[id].parent_id = current_process;
			process_table[id].status = PROC_ALIVE;
			process_table[id].priority = process_table[current_process].priority;
			process_table[id].uid = process_table[current_process].uid;
			process_table[id].suid = process_table[current_process].suid;
			process_table[id].euid = process_table[current_process].euid;
			process_table[id].is_driver = process_table[current_process].is_driver;
			return id;
		}
	}

	return 0;
}

u32 release_process(u32 process_id)
{
	u32 exit_code;
	plist_node_t p_node;

	//Release all zombie threads
	p_node = process_table[current_process].zombie_list;

	while(p_node != NULL) {
		release_thread((u32)(p_node->p_item));
		rtl_list_remove(&(process_table[process_id].zombie_list),
		                p_node,
		                process_heap);
		p_node = process_table[current_process].zombie_list;
	}

	//Free pdt
	mm_pg_tbl_free(process_table[process_id].pdt_id);

	//Release process name and root path
	mm_hp_free(process_table[process_id].process_name, process_heap);

	//Get exit code
	exit_code = process_table[process_id].exit_code;

	//Clear allocate flag
	process_table[process_id].alloc_flag = false;
	return exit_code;
}

void awake_threads(list_t lst, u32 status)
{
	plist_node_t p_node;

	p_node = lst;

	if(p_node != NULL) {
		do {
			if(thread_table[(u32)p_node->p_item].status == status) {
				pm_resume_thrd((u32)p_node->p_item);
			}

			p_node = p_node->p_next;
		} while(p_node != lst);
	}

	return;
}
