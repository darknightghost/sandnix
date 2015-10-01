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

#ifndef	MESSAGES_H_INCLUDE
#define	MESSAGES_H_INCLUDE

#include "../../common/common.h"
#include "../vfs/vfs.h"
#include "../mm/mm.h"
#include "../rtl/rtl.h"

//Types
typedef	struct	_msg {
	u32				message;
	u32				status;
	u32				msg_id;
	u32				src_thread;
	u32				result_queue;
	u32				file_id;
	k_status		result;
	size_t			size;
	union {
		u32	flags;
		struct {
			u32		direct_buf: 1;
			u32		pmo_buf: 1;
			u32		async: 1;
			u32		broadcast: 1;
		} properties;
	} flags;
	union {
		void*	addr;		//Don't too large,slow,one direction only
		ppmo_t	pmo_addr;	//Fast,large buf should use this,can get return value
	} buf;
} msg_t, *pmsg_t;


typedef	struct {
	queue_t	msgs;
	mutex_t	lock;
	u32		blocked_thread_id;
	bool	destroy_flag;
} msg_queue_t, *pmsg_queue_t;

typedef	struct {
	u32			msg_id;
	k_status	result;
} msg_complete_info_t, *pmsg_complete_info_t;

typedef	struct {
	u32		msg_id;
} msg_cancel_info_t, *pmsg_cancel_info_t;

typedef	struct {
	u32			process;
	u32			flags;
	u32			mode;
	u32			file_object;
	u64			file_size;
	bool		serial_read;
	char		path;
} msg_open_info_t, *pmsg_open_info_t;

typedef	struct {
	u32		file_obj_id;
} msg_close_info_t, *pmsg_close_info_t;

typedef	struct {
	u32		file_obj;
	u64		offset;
	size_t	len;		//Caller should fill
} msg_read_info_t, *pmsg_read_info_t;

typedef	struct {
	size_t	len;
	u8		data;
} msg_read_data_t, *pmsg_read_data_t;

typedef	struct {
	u32		file_obj;
	u64		offset;
	size_t	count;		//Caller should fill
} msg_readdir_info_t, *pmsg_readdir_info_t;

typedef	struct {
	size_t		count;
	u8			data;
} msg_readdir_data_t, *pmsg_readdir_data_t;

typedef	struct {
	u32		file_obj;
	u64		offset;
	size_t	len;		//Caller should fill
	u8		data;
} msg_write_info_t, *pmsg_write_info_t;

typedef	struct {
	u32		mode;
	u32		process;
	char	path;
} msg_access_info_t, *pmsg_access_info_t;

typedef	struct {
	char	path;
} msg_stat_info_t, *pmsg_stat_info_t;

typedef struct _file_stat_t	file_stat_t, *pfile_stat_t;

typedef	struct {
	file_stat_t		stat;
} msg_stat_data_t, *pmsg_stat_data_t;

typedef	struct {
	u32		mode;
	u32		process;
} msg_chmod_info_t, *pmsg_chmod_info_t;

typedef	struct {
	u32		process;
	char	path;
} msg_unlink_info_t, *pmsg_unlink_info_t;

typedef	struct {
	u32		process;
	u32		mode;
	char	path;
} msg_mkdir_info_t, *pmsg_mkdir_info_t;

typedef	struct {
	u32		volume_dev;
	u32		flags;
	u32		path_offset;
	u32		args_offset;
	u32		mode;
	u8		data;
} msg_mount_info_t, *pmsg_mount_info_t;

typedef	struct {
	u32		volume_dev;
} msg_umount_info_t, *pmsg_umount_info_t;

#endif	//!	MESSAGES_H_INCLUDE
