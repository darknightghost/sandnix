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

spinlock_t		process_table_lock;
process_info_t	process_table[MAX_PROCESS_NUM];
void*			process_heap;


static	u32			get_free_proc_id();
static	u32			fork_descrpitor_list(u32 dest, u32 src);
static	void		descriptor_destroy_callback(void* descriptor, void* p_arg);
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
	process_table[0].root_path = "/";
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
	if(fork_descrpitor_list(new_id, current_process) != ESUCCESS) {
		process_table[new_id].alloc_flag = false;
		mm_pg_tbl_free(new_pdt);
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ENOMEM);
		return -1;
	}

	//Fork thread
	new_thread = fork_thread(new_id);

	if(new_thread < 0) {
		rtl_list_destroy(&(process_table[new_id].file_desc_list),
		                 process_heap,
		                 descriptor_destroy_callback,
		                 NULL);
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

void pm_exec(char* cmd_line, char* image_path)
{
	//TODO:exec
	UNREFERRED_PARAMETER(cmd_line);
	UNREFERRED_PARAMETER(image_path);
}

u32 pm_wait(u32 child_id, bool if_block)
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
	while(if_block) {
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
				   (u32)(p_node->p_item) != child_id) {
					//Return
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

		if(!pm_should_break()) {
			wait_thread();
		}

		pm_acqr_spn_lock(&process_table_lock);

	}

	//Clear zombie threads
	pm_rls_spn_lock(&process_table_lock);

	return 0;
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

bool pm_add_proc_file_descriptor(u32 process_id, u32 descriptor)
{
	pm_acqr_spn_lock(&process_table_lock);

	if(process_table[process_id].alloc_flag == false) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ESRCH);
		return false;
	}

	//Add new file descriptor
	if(rtl_list_insert_after(&(process_table[process_id].file_desc_list),
	                         NULL,
	                         (void*)descriptor,
	                         process_heap) == NULL) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(EFAULT);
		return false;
	}

	pm_rls_spn_lock(&process_table_lock);

	pm_set_errno(ESUCCESS);
	return true;
}

bool pm_remove_proc_file_descriptor(u32 process_id, u32 descriptor)
{
	plist_node_t p_node;

	pm_acqr_spn_lock(&process_table_lock);

	if(process_table[process_id].alloc_flag == false) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ESRCH);
		return false;
	}

	//Remove from descriptor list_t
	p_node = rtl_list_get_node_by_item(process_table[process_id].file_desc_list,
	                                   (void*)descriptor);

	if(p_node == NULL) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(EFAULT);
		return false;
	}

	rtl_list_remove(&(process_table[process_id].file_desc_list),
	                p_node,
	                process_heap);

	pm_rls_spn_lock(&process_table_lock);
	pm_set_errno(ESUCCESS);

	return true;
}

bool pm_chroot(char* new_root)
{
	size_t path_len;

	pm_acqr_spn_lock(&process_table_lock);

	mm_hp_free(process_table[current_process].root_path, process_heap);

	path_len = rtl_strlen(new_root) + 1;
	process_table[current_process].root_path = mm_hp_alloc(
	            path_len,
	            process_heap);
	rtl_strcpy_s(process_table[current_process].root_path,
	             path_len,
	             new_root);

	pm_rls_spn_lock(&process_table_lock);

	pm_set_errno(ESUCCESS);

	return true;

}

size_t pm_get_root(u32 process_id, size_t buf_size, char* buf)
{
	size_t path_len;

	pm_acqr_spn_lock(&process_table_lock);

	//Check if the process exists
	if(!process_table[process_id].alloc_flag) {
		pm_rls_spn_lock(&process_table_lock);
		pm_set_errno(ESRCH);
		return 0;
	}

	path_len = rtl_strlen(process_table[current_process].root_path) + 1;
	rtl_strcpy_s(buf,
	             buf_size,
	             process_table[current_process].root_path);
	pm_rls_spn_lock(&process_table_lock);

	pm_set_errno(ESUCCESS);

	return path_len;

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
		rtl_list_destroy(&(process_table[proc_id].file_desc_list),
		                 process_heap,
		                 descriptor_destroy_callback,
		                 NULL);

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
	size_t path_len;

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
			path_len = rtl_strlen(process_table[current_process].root_path) + 1;
			process_table[id].root_path = mm_hp_alloc(
			                                  path_len,
			                                  process_heap);
			rtl_strcpy_s(process_table[id].root_path,
			             path_len,
			             process_table[current_process].root_path);
			process_table[id].parent_id = current_process;
			process_table[id].status = PROC_ALIVE;
			process_table[id].priority = process_table[current_process].priority;
			process_table[id].uid = process_table[current_process].uid;
			process_table[id].suid = process_table[current_process].suid;
			process_table[id].euid = process_table[current_process].euid;
			return id;
		}
	}

	return 0;
}

u32 fork_descrpitor_list(u32 dest, u32 src)
{
	plist_node_t p_node;

	p_node = process_table[src].file_desc_list;

	if(p_node != NULL) {
		do {
			//Increase reference count
			//TODO:
			//	vfs_inc_fdesc_reference((u32)(p_node->p_item));

			//Add to child's descriptor list_t
			if(rtl_list_insert_after(&(process_table[dest].file_desc_list),
			                         NULL,
			                         p_node->p_item,
			                         process_heap) == NULL) {
				rtl_list_destroy(&(process_table[dest].file_desc_list),
				                 process_heap,
				                 descriptor_destroy_callback,
				                 NULL);
				return EAGAIN;

			}

			p_node = p_node->p_next;
		} while(p_node != process_table[src].file_desc_list);
	}

	return ESUCCESS;
}

void descriptor_destroy_callback(void* descriptor, void* p_arg)
{
	//TODO:
	//	vfs_close((u32)descriptor);
	UNREFERRED_PARAMETER(descriptor);
	UNREFERRED_PARAMETER(p_arg);
	return;
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
	mm_hp_free(process_table[process_id].root_path, process_heap);

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
