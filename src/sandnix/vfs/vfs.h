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

#define	MAX_FILEOBJ_NUM		2048
#define	NAME_MAX			255
#define	PATH_MAX			2048

//Open flags
#define	O_RDONLY	0x00000000
#define	O_WRONLY	0x00000001
#define	O_RDWR		0x00000002

#define	O_CREAT		0x00000040
#define	O_EXCL		0x00000080
#define	O_NOCTTY	0x00000100
#define	O_TRUNC		0x00000200	//Not support
#define	O_APPEND	0x00000400
#define	O_NONBLOCK	0x00000800	//Not support
#define	O_NDELAY	O_NONBLOCK
#define	O_DSYNC		0x00001000	//Not support
#define	O_ASYNC		0x00002000	//Not support
#define	O_DIRECTORY	0x00010000
#define	O_NOFOLLOW	0x00020000
#define	O_CLOEXEC	0x00080000
#define	O_RSYNC		0x00101000	//Not support
#define	O_SYNC		0x00101000	//Not support

//Modes
#define	S_ISUID	0x00000800		//Set user ID
#define	S_ISGID	0x00000400		//Set group ID

#define	S_ISVTX	0x00000200		//Sticky bit

#define	S_IRWXU	0x000001c0		//Owner has read,write&execute permissions
#define	S_IRUSR	0x00000100		//Owner has read permission
#define	S_IWUSR	0x00000080		//Owner has write permission
#define	S_IXUSR	0x00000040		//Owner has execute permission

#define	S_IRWXG	0x00000038		//Group has read,write&execute permissions
#define	S_IRGRP	0x00000020		//Group has read permission
#define	S_IWGRP	0x00000010		//Group has write permission
#define	S_IXGRP	0x00000008		//Group has execute permission

#define	S_IRWXO	0x00000007		//Others has read,write&execute permissions
#define	S_IROTH	0x00000004		//Others has read permission
#define	S_IWOTH	0x00000002		//Others has write permission
#define	S_IXOTH	0x00000001		//Others has execute permission

//Access modes
#define	F_OK	0x00000000		//Read
#define	X_OK	0x00000001		//Execute
#define	W_OK	0x00000002		//Write
#define	R_OK	0x00000004		//Exist

//Seek
#define	SEEK_SET	0x00000000
#define	SEEK_CUR	0x00000001
#define	SEEK_END	0x00000002

#include "om/om.h"
#include "fs/fs.h"
#include "../msg/msg.h"

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
k_status		vfs_access(char* path, u32 mode);
void			vfs_close(u32 fd);
k_status		vfs_read(u32 fd, ppmo_t buf);
size_t			vfs_write(u32 fd, ppmo_t buf);
s64				vfs_seek(u32 fd, u32 pos, s64 offset);
k_status		vfs_stat(char* path, ppmo_t buf);
k_status		vfs_remove(char* path);
k_status		vfs_mkdir(char* path, u32 mode);
k_status		vfs_readdir(u32 fd, ppmo_t buf);
void			vfs_sync(u32 dev_num);
//s32			vfs_ioctl(u32 fd, u32 request, ...);

//File object
u32				vfs_create_file_object();
k_status		vfs_send_file_message(u32 src_driver,
                                      u32 dest_file_object,
                                      pmsg_t p_msg,
                                      u32* p_result,
                                      k_status* p_complete_result);
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
                                     u32* p_result,
                                     k_status* p_complete_result);
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
                                     u32* p_result,
                                     k_status* p_complete_result);
u32				vfs_get_dev_major_by_name(char* major_name, u32 type);
k_status		vfs_msg_forward(pmsg_t p_msg, u32 dev_num);
void			vfs_sync(u32 dev_num);

#endif	//!	VFS_H_INCLUDE
