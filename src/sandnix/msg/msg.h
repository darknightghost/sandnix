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

#ifndef	MSG_H_INCLUDE
#define	MSG_H_INCLUDE

#include "../../common/common.h"
#include "../mm/mm.h"
#include "../rtl/rtl.h"
#include "../vfs/vfs.h"

#define	MAX_MSG_QUEUE_NUM		65535

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
	size_t		file_size;
	bool		serial_read;
	char		path;
} msg_open_info_t, *pmsg_open_info_t;

typedef	struct {
	u32		file_obj_id;
} msg_close_info_t, *pmsg_close_info_t;

typedef	struct {
	u32		file_obj;
	size_t	offset;
	size_t	len;		//Caller should fill
} msg_read_info_t, *pmsg_read_info_t;

typedef	struct {
	size_t	len;
	u8		data;
} msg_read_data_t, *pmsg_read_data_t;

typedef	struct {
	u32		file_obj;
	size_t	offset;
	size_t	count;		//Caller should fill
} msg_readdir_info_t, *pmsg_readdir_info_t;

typedef	struct {
	size_t		count;
	u8			data;
} msg_readdir_data_t, *pmsg_readdir_data_t;

typedef	struct {
	u32		file_obj;
	size_t	offset;
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
} msg_remove_info_t, *pmsg_remove_info_t;

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

//Flags
#define		MFLAG_DIRECTBUF		0x00000001
#define		MFLAG_PMO			0x00000002
#define		MFLAG_ASYNC			0x00000004
#define		MFLAG_BROADCAST		0x00000008

//Messages
#define		MSG_COMPLETE		0x00000000
#define		MSG_CANCEL			0x00000001
#define		MSG_FAILED			0x00000002
#define		MSG_OPEN			0x00000003
#define		MSG_READ			0x00000004
#define		MSG_WRITE			0x00000005
#define		MSG_REMOVE			0x00000006
#define		MSG_MKDIR			0x00000007
#define		MSG_READDIR			0x00000008
#define		MSG_CLOSE			0x00000009
#define		MSG_DESTROY			0x0000000A
#define		MSG_ACCESS			0x0000000B
#define		MSG_STAT			0x0000000C
#define		MSG_CHOMD			0x0000000D
#define		MSG_IOCTRL			0x0000000E
#define		MSG_INTERRUPT		0x0000000F
#define		MSG_MOUNT			0x00000010
#define		MSG_UMOUNT			0x00000011

//Status
#define		MSTATUS_COMPLETE	0x00000000
#define		MSTATUS_CANCEL		0x00000001
#define		MSTATUS_FORWARD		0x00000002

void		msg_init();

//Message queue
u32			msg_queue_create();
void		msg_queue_destroy(u32 id);

//Mesage send&recv
k_status	msg_create(pmsg_t *p_p_msg, size_t size);
k_status	msg_send(pmsg_t p_msg,
                     u32 dest_queue,
                     u32* p_result,
                     k_status* p_complete_status);
k_status	msg_recv(pmsg_t* p_p_msg, u32 dest_queue, bool if_block);

//Message dealing
k_status	msg_forward(pmsg_t p_msg, u32 dest_queue);
k_status	msg_complete(pmsg_t p_msg, k_status result);
k_status	msg_cancel(pmsg_t p_msg);




#endif	//!	MSG_H_INCLUDE


