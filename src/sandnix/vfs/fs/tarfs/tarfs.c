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

#include "tarfs.h"
#include "../../vfs.h"
#include "../../../debug/debug.h"
#include "../../../rtl/rtl.h"
#include "tar.h"
#include "fs_structs.h"

u32		initrd_volume;
u32		initrd_fs;

static	array_list_t	inode_table;

static	void	kdriver_main(u32 thread_id, void* p_null);
static	bool	dispatch_message(pmsg_t p_msg);
static	void	on_mount(pmsg_t p_msg);
static	void	on_umount(pmsg_t p_msg);
static	void	on_open(pmsg_t p_msg);
static	void	on_read(pmsg_t p_msg);
static	void	on_write(pmsg_t p_msg);
static	void	on_close(pmsg_t p_msg);

void tarfs_init()
{
	dbg_print("Initializing tarfs...\n");
	
	//Create driver process
	if(pm_fork() == 0) {
		pm_exec("initrd", NULL);
		pm_clear_kernel_stack(kdriver_main, NULL);

	} else {
		pm_suspend_thrd(pm_get_crrnt_thrd_id());
	}

	return;
}

void kdriver_main(u32 thread_id, void* p_null)
{
	//Analyse inodes
}


bool dispatch_message(pmsg_t p_msg);