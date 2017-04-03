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

#include "../../../../../common/common.h"
#include "./messages/msg_obj_defs.h"
#include "../../rtl/rtl_defs.h"
#include "../../pm/pm_defs.h"

typedef	struct	_msg_queue_obj {
    obj_t			obj;			//Base class

    queue_t			msg_queue;		//Messages
    u32				msg_count;		//How many messages in the queue
    mutex_t			lock;			//Queue lock
    cond_t			msg_cond;		//Used to wait for message

    bool			alive;			//If the message queue is alive

    //Methods
    //kstatus_t		send(pmsg_queue_obj_t p_this, pmsg_obj_t p_msg,
    //			mstatus_t* result);
    kstatus_t	(*send)(struct _msg_queue_obj*, pmsg_obj_t, mstatus_t*);

    //kstatus_t		recv(pmsg_queue_obj_t p_this, pmsg_obj_t* p_ret,
    //			s32 millisec_timeout);
    kstatus_t	(*recv)(struct _msg_queue_obj*, pmsg_obj_t*, s32);

    //u32			get_msg_count(pmsg_queue_obj_t p_this);
    u32(*get_msg_count)(struct _msg_queue_obj*);

    //void			destroy(pmsg_queue_obj_t p_this);
    void	(*destroy)(struct _msg_queue_obj*);

} msg_queue_obj_t, *pmsg_queue_obj_t;
