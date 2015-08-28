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

#include "fs.h"
#include "../../pm/pm.h"
#include "../../rtl/rtl.h"
#include "../../exceptions/exceptions.h"
#include "ramdisk/ramdisk.h"
#include "tarfs/tarfs.h"
#include "../../debug/debug.h"

static	array_list_t	file_desc_info_table;
static	mutex_t			file_desc_info_table_lock;
static	mount_point_t	root_info;
static	mutex_t			mount_point_lock;

void fs_init()
{
	k_status status;
	dbg_print("Initializing filesystem...\n");

	//Initialize file descriptors table
	status = rtl_array_list_init(&file_desc_info_table,
	                             MAX_PROCESS_NUM,
	                             NULL);

	if(status != ESUCCESS) {
		excpt_panic(status, "Failed to initialize file_desc_info_table.");
	}

	pm_init_mutex(&file_desc_info_table_lock);
	pm_init_mutex(&mount_point_lock);

	//Initialize ramdisk and tarfs
	ramdisk_init();
	tarfs_init();

	//Mount ramdisk as root filesytem
}

//Volumes
k_status		vfs_mount(char* src, char* target,
                          char* fs_type, u32 flags,
                          char* args);
k_status		vfs_umount(char* path);

//Path
k_status		vfs_chroot(char* path);
k_status		vfs_chdir(char* path);

//File descriptors
k_status vfs_fork(u32 dest_process)
{
	UNREFERRED_PARAMETER(dest_process);
	return ESUCCESS;
}

void vfs_clean(u32 process_id)
{
	UNREFERRED_PARAMETER(process_id);
	return;
}

//Files
u32				vfs_open(char* path, u32 flags, u32 mode);
k_status		vfs_chmod(u32 fd, u32 mode);
bool			vfs_access(char* path, u32 mode);
bool			vfs_close(u32 fd);
size_t			vfs_read(u32 fd, void* buf, size_t count);
size_t			vfs_write(u32 fd, void* buf, size_t count);
void			vfs_sync();
bool			vfs_syncfs(u32 volume_dev);
//s32			vfs_ioctl(u32 fd, u32 request, ...);
