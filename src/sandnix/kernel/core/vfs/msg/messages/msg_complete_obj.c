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

#include "../../../../../../common/common.h"
#include "./msg_obj.h"
#include "./msg_complete_obj.h"
#include "../../../exception/exception.h"
#include "../../../rtl/rtl.h"
#include "../../../pm/pm.h"

static	void	complete(pmsg_obj_t p_this, bool success);
static	void	cancel(pmsg_obj_t p_this);

pmsg_complete_obj_t msg_complete_obj(
    pmsg_obj_t p_completed_msg,
    mstatus_t status,
    pheap_t heap)
{
    pmsg_complete_obj_t p_ret = (pmsg_complete_obj_t)msg_obj(
                                    MSG_MJ_COMPLETED,
                                    0,
                                    MSG_ATTR_ASYNC,
                                    NULL,
                                    sizeof(msg_complete_obj_t),
                                    heap);

    if(p_ret == NULL) {
        return NULL;
    }

    //Variables
    p_ret->p_completed_msg = p_completed_msg;
    p_ret->status = status;

    //Methods
    p_ret->msg.complete = complete;
    p_ret->msg.cancel = cancel;

    return p_ret;
}

void complete(pmsg_obj_t p_this, bool success)
{
    core_pm_mutex_acquire(&(p_this->lock), -1);

    //Check status
    if(p_this->status != MSG_STATUS_PENDING) {
        core_pm_mutex_release(&(p_this->lock));
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal message status!");
        return;
    }

    //Change status
    if(success) {
        p_this->status = MSG_STATUS_SUCCESS;

    } else {
        p_this->status = MSG_STATUS_FAILED;
    }

    core_pm_mutex_release(&(p_this->lock));

    return;
}

void cancel(pmsg_obj_t p_this)
{
    core_pm_mutex_acquire(&(p_this->lock), -1);

    //Check status
    if(p_this->status != MSG_STATUS_PENDING) {
        core_pm_mutex_release(&(p_this->lock));
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal message status!");
        return;
    }

    //Change status
    p_this->status = MSG_STATUS_CANCELED;

    core_pm_mutex_release(&(p_this->lock));

    return;
}
