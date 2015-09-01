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
#include "fs/fs.h"

#define	MAX_FILEOBJ_NUM		2048

typedef	struct	_msg	msg_t, *pmsg_t;

void			vfs_init();

//Volumes
k_status		vfs_mount(char* src, char* target,
                          char* fs_type, u32 flags,
                          char* args);
k_status		vfs_umount(char* path);

//Path
k_status		vfs_chroot(char* path);
k_status		vfs_chdir(char* path);

//File descriptors
k_status		vfs_fork(u32 dest_process);
void			vfs_clean(u32 process_id);

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

//File object
u32				vfs_create_file_object(u32 driver);
k_status		vfs_send_file_message(u32 src_driver,
                                      u32 dest_file_object,
                                      pmsg_t p_msg,
                                      u32* p_result);
//Objects
void			vfs_initialize_object(pkobject_t p_object);
void			vfs_inc_obj_reference(pkobject_t p_object);
void			vfs_dec_obj_reference(pkobject_t p_object);

//Driver Objects
pdriver_obj_t	vfs_create_drv_object(char* drv_name);
u32				vfs_reg_driver(pdriver_obj_t p_driver);
k_status		vfs_send_drv_message(u32 src_driver,
                                     u32 dest_driver,
                                     pmsg_t p_msg,
                                     u32* p_result);
k_status		vfs_recv_drv_message(u32 drv_num, pmsg_t* p_p_msg,
                                     bool if_block);

//Device objects
pdevice_obj_t	vfs_create_dev_object(char* dev_name);
u32				vfs_add_device(pdevice_obj_t p_device, u32 driver);
void			vfs_remove_device(u32 device);
k_status		vfs_set_dev_filename(u32 device, char* name);
k_status		vfs_send_dev_message(u32 src_driver,
                                     u32 dest_dev,
                                     pmsg_t p_msg,
                                     u32* p_result);
u32				vfs_get_dev_major_by_name(char* major_name, u32 type);
k_status		vfs_msg_forward(pmsg_t p_msg);

#endif	//!	VFS_H_INCLUDE
