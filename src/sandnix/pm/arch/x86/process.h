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

#ifndef	PROCESS_H_INCLUDE
#define	PROCESS_H_INCLUDE

#include "../../pm.h"

typedef struct _process_info {
	bool		alloc_flag;			//Allocate flag
	char*		process_name;		//Name
	bool		wait_flag;			//Is been waited
	u32			parent_id;			//Parent process
	u32			thread_num;			//Number of threads
	u32			pdt_id;				//Page table id
	u32			exit_code;			//Exit code
	u32			status;				//Status,alive or zombie
	u32			priority;			//0x00-0x0F
	u32			uid;				//Real uid
	u32			suid;				//Saved uid
	u32			euid;				//Effective uid
	u32			gid;				//Real gid
	u32			sgid;				//Saved gid
	u32			egid;				//Effective gid
	u32			is_driver;			//The process is a driver when it is set
	list_t		child_list;			//Child processes
	list_t		thread_list;		//Alive threads
	list_t		zombie_list;		//Zombie threads
	list_t		wait_list;			//Zombie children
} process_info_t, *pprocess_info_t;


extern	spinlock_t		process_table_lock;
extern	process_info_t	process_table[MAX_PROCESS_NUM];
extern	void*			process_heap;

void	init_process();
void	set_exit_code(u32 process, u32 exit_code);
u32		get_process_pdt(u32 process_id);
void	switch_process(u32 process_id);
void	add_proc_thrd(u32 thrd_id, u32 proc_id);
void	zombie_proc_thrd(u32 thrd_id, u32 proc_id);

#endif	//!	PROCESS_H_INCLUDE
