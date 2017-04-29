/*
    Copyright 2017,王思远 <darknightghost.cn@gmail.com>

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

#pragma once

#include "../../../../common/common.h"
#include "./msg/msg_queue_obj_defs.h"
#include "../rtl/rtl_defs.h"
#include "../pm/pm_defs.h"

#define	MODULE_NAME		core_vfs

#ifndef	MAX_FILEOBJ_NUM
    #define	MAX_FILEOBJ_NUM		8192
#endif

//File object
typedef struct	_file_obj {
    //Parent
    obj_t		obj;

    //Attributes
    u32					file_obj_id;//Id of the file object.
    mutex_t				lock;		//File object lock.
    bool				alive;		//If the file object cannot be access, set it to false.
    u32					inode;		//Inode number of the file.
    pmsg_queue_obj_t	msg_queue;	//Message queue.

    //Methods
    //This method is used to set the alive flag to false and destroy the message queue.
    //void		destroy(pfile_obj_t p_this);
    void	(*destroy)(struct _file_obj*);

    //This method is used to send message.

    //Protected methods
    //Lock
    //void			lock_obj(pfile_obj_t p_this);
    void	(*lock_obj)(struct _file_obj*);

    //void			unlock_obj(pfile_obj_t p_this);
    void	(*unlock_obj)(struct _file_obj*);
} file_obj_t, *pfile_obj_t;

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
#define	F_OK	0x00000000		//Exist
#define	X_OK	0x00000001		//Execute
#define	W_OK	0x00000002		//Write
#define	R_OK	0x00000004		//Read

#undef	MODULE_NAME
