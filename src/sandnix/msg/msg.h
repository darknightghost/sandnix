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
#include "messages.h"

#define	MAX_MSG_QUEUE_NUM		65535

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
#define		MSG_UNLINK			0x00000006
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
