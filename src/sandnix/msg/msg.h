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

//Types
typedef	u32		m_status;

typedef	struct	_msg {
	u32		message;
	u32		status;
	u32		msg_id;
	u32		src_thrd_id;
	u32		dest_thrd_id;
	union {
		u32	flags;
		struct {
			u32		buf_type: 2;
			u32		async: 1;
			u32		broadcast: 1;
		} properties;
	} flags;
	union {
		struct {
			void*	addr;
			size_t	size;
		} buf;
		ppmo	pmo_addr;
	} buf;
} msg, *pmsg;


typedef	struct {
	queue	msgs;
	mutex	lock;
	u32		blocked_thread_id;
} msg_queue, *pmsg_queue;

//Flags
#define		MFLAG_DIRECTBUF		0x00000001
#define		MFLAG_PMO			0x00000002
#define		MFLAG_ASYNC			0x00000004
#define		MFLAG_BROADCAST		0x00000008

//Messages
#define		MSG_COMPLETE		0x00000000
#define		MSG_OPEN			0x00000001
#define		MSG_READ			0x00000002
#define		MSG_WRITE			0x00000003
#define		MSG_CLOSE			0x00000004
#define		MSG_DESTROY			0x00000005
#define		MSG_IOCTRL			0x00000006
#define		MSG_INTERRUPT		0x00000007


//Status
#define		MSTATUS_COMPLETE	0x00000000
#define		MSTATUS_PENDING		0x00000001
#define		MSTATUS_CANCEL		0x00000002
#define		MSTATUS_FORWARD		0x00000003

void		msg_init();

//msg_queue
u32			msg_queue_create();
void		msg_queue_destroy(u32 id);

//Mesage send&recv
k_status	msg_create(pmsg p_msg);
m_status	msg_send(pmsg p_msg, u32 dest);
m_status	msg_recv(pmsg buf, u32 dest);

//Message dealing
k_status	msg_forward(pmsg p_msg);
k_status	msg_pending(pmsg p_msg);
k_status	msg_complete(pmsg p_msg);
k_status	msg_cancel(pmsg p_msg);




#endif	//!	MSG_H_INCLUDE
