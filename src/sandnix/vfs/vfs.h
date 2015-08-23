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

#ifndef	VFS_H_INCLUDE
#define	VFS_H_INCLUDE

#include "../../common/common.h"
#include "./om/om.h"
#include "./fs/fs.h"
#include "../msg/msg.h"

typedef	struct	_msg	msg_t, *pmsg_t;

void			vfs_init();

//Devices
k_status		vfs_mount(char* src, char* target,
                          char* fs_type, u32 flags,
                          char* args);
k_status		vfs_umount(char* path);

//Files
pfile_obj_t		vfs_open(char* path, u32 flags, u32 mode);
k_status		vfs_chmod(pfile_obj_t fo, u32 mode);
bool			vfs_access(char* path, u32 mode);
bool			vfs_close(pfile_obj_t fo);
size_t			vfs_read(pfile_obj_t fo, void* buf, size_t offset, size_t count);
size_t			vfs_write(pfile_obj_t fo, void* buf, size_t offset, size_t count);
void			vfs_sync();
bool			vfs_syncfs(pfile_obj_t fo);
s32				vfs_ioctl(pfile_obj_t fo, u32 request, ...);

//Objects
void			vfs_inc_obj_reference(pkobject_t p_object);
void			vfs_dec_obj_reference(pkobject_t p_object);

//Driver Objects
u32				vfs_reg_driver(pdriver_obj_t p_driver);
void			vfs_unreg_driver(u32 driver_id);
k_status		vfs_send_drv_message(u32 dest_driver,
                                     pmsg_t p_msg);
k_status		vfs_recv_drv_message(pmsg_t buf);

//Device objects
u32				vfs_add_device(pdevice_obj_t p_device, u32 driver);
void			vfs_remove_device(u32 device);

k_status		vfs_send_dev_message(u32 dest_dev,
                                     pmsg_t p_msg);
u32				vfs_get_dev_major(char* major_name);

#endif	//!	VFS_H_INCLUDE
