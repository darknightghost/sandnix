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

#include "../../../../../../common/common.h"
#include "../../../rtl/rtl_defs.h"
#include "../../../mm/mm_defs.h"

struct	_msg_queue_obj;

typedef	struct	_msg_obj {
    //Object
    obj_t	obj;			//Base class

    //Message attributes
    u32		major_type;		//Major type
    u32		minor_type;		//Minor type
    u32		status;			//Message status
    u32		attr;			//Message attributes
    struct	_msg_queue_obj*	p_reply_queue;	//Reply queue, only used in async messages
    union {
        struct {
            void*			p_buf;		//Address of buffer
            size_t			size;		//Buffer size
        } by_buffer;

        struct {
            ppage_obj_t		p_page_obj;	//Page object
        } by_page_obj;

    } data;					//Message data

    //Methods
    //Complete message
    //Cancel message
} msg_obj_t, *pmsg_obj_t;

#include "../msg_queue_obj_defs.h"

//Message status
#define MSG_STATUS_SUCCESS		0x00000000
#define MSG_STATUS_FAILED		0x00000001
#define MSG_STATUS_PENDING		0x00000002
#define MSG_STATUS_CANCEL		0x00000003

//Message attributes
#define	MSG_ATTR_ASYNC			0x00000001
#define	MSG_ATTR_BY_BUF			0x00000002
#define	MSG_ATTR_BY_PAGE_OBJ	0x00000004

//Message types
//Major types
//Async message
#define	MSG_MJ_FINISH		0x00000000			//Message finished

//File operations
#define MSG_MJ_OPEN			0x00000010
#define MSG_MJ_READ			0x00000011
#define MSG_MJ_WRITE		0x00000012
#define MSG_MJ_TRUNCATE		0x00000013
#define MSG_MJ_CLOSE		0x00000014
#define MSG_MJ_STAT			0x00000015
#define MSG_MJ_FCNTL		0x00000016
#define MSG_MJ_LINK			0x00000017
#define MSG_MJ_CHMOD		0x00000018
#define MSG_MJ_CHOWN		0x00000019
#define MSG_MJ_MKDIR		0x0000001A
#define MSG_MJ_ACCESS		0x0000001B
#define MSG_MJ_MKNOD		0x0000001C
#define MSG_MJ_IOCTL		0x0000001D
#define MSG_MJ_NOTIFY		0x0000001E
#define MSG_MJ_MOUNT		0x0000001F

//Device operations
#define MSG_MJ_MATCH			0x00000030
#define MSG_MJ_HOT_PLUG			0x00000031
#define MSG_MJ_POWER			0x00000032

//Minor types
//MSG_MJ_FINISH
#define MSG_MN_COMPLETE			0x00000000
#define MSG_MN_CANCEL			0x00000001

//MSG_MJ_OPEN
#define MSG_MN_OPEN				0x00000000

//MSG_MJ_READ
#define MSG_MN_READ				0x00000000

//MSG_MJ_WRITE
#define MSG_MN_WRITE			0x00000000

//MSG_MJ_TRUNCATE
#define MSG_MN_TRUNCATE			0x00000000

//MSG_MJ_CLOSE
#define MSG_MN_CLOSE			0x00000000
#define MSG_MN_CLEANUP			0x00000001

//MSG_MJ_STAT
#define MSG_MN_STAT				0x00000000

//MSG_MJ_FCNTL
#define MSG_MN_FCNTL			0x00000000

//MSG_MJ_LINK
#define MSG_MN_SYMBOL_LINK		0x00000000
#define MSG_MN_HARD_LINK		0x00000001
#define MSG_MN_UNLINK			0x00000002

//MSG_MJ_CHMOD
#define MSG_MN_CHMOD			0x00000000

//MSG_MJ_CHOWN
#define MSG_MN_CHOWN			0x00000000

//MSG_MJ_MKDIR
#define MSG_MN_MKDIR			0x00000000

//MSG_MJ_ACCESS
#define MSG_MN_ACCESS			0x00000000

//MSG_MJ_MKNOD
#define MSG_MN_MKNOD			0x00000000

//MSG_MJ_IOCTL
#define MSG_MN_IOCTL			0x00000000

//MSG_MJ_NOTIFY
#define MSG_MN_NOTIFY			0x00000000
#define MSG_MN_NOTIFY_ADD		0x00000001
#define MSG_MN_NOTIFY_REMOVE	0x00000002

//MSG_MJ_MOUNT
#define MSG_MN_MOUNT			0x00000000
#define MSG_MN_UMOUNT			0x00000001

//MSG_MJ_MATCH
#define MSG_MN_MATCH			0x00000000
#define MSG_MN_UNMATCH			0x00000001

//MSG_MJ_HOT_PLUG
#define MSG_MN_PLUGIN			0x00000000
#define MSG_MN_PLUGOFF			0x00000001

//MSG_MJ_POWER
#define MSG_MN_POWEROFF			0x00000000
#define MSG_MN_SUSPEND			0x00000001
#define MSG_MN_RESUME			0x00000002
#define MSG_MN_HIBERNATE		0x00000003
