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
	u32			parent_id;			//Parent process
	u32			pdt_id;				//Page table id
	u32			exit_code;			//Exit code
	u32			status;				//Status,alive or zombie
	u32			priority;			//0x00-0x0F
	u32			uid;				//Real uid
	u32			euid;				//Effective uid
	spin_lock	child_list_lock;
	list		child_list;			//Child processes
	spin_lock	thread_list_lock;
	list		thread_list;		//Alive threads
	spin_lock	zombie_list_lock;
	list		zombie_list;		//Zombie threads
	spin_lock	wait_list_lock;
	list		wait_list;			//Zombie children
} process_info, *pprocess_info;

void	add_proc_thrd(u32 thrd_id, u32 proc_id);
void	zomble_proc_thrd(u32 thrd_id, u32 proc_id);
void	remove_proc_thrd(u32 thrd_id, u32 proc_id);
void	init_process();

#endif	//!	PROCESS_H_INCLUDE
