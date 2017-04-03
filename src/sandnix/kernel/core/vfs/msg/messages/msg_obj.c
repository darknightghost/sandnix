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
#include "../../../exception/exception.h"
#include "../../../rtl/rtl.h"
#include "../../../pm/pm.h"

#include "../msg_queue_obj.h"
#include "./msg_obj.h"
#include "./msg_complete_obj.h"
#include "./msg_cancel_obj.h"

static	void			destructor(pmsg_obj_t p_this);
static	int				compare(pmsg_obj_t p_this, pmsg_obj_t p_obj);
static	pkstring_obj_t	to_string(pmsg_obj_t p_this);

static	void	complete(pmsg_obj_t p_this, bool success);
static	void	cancel(pmsg_obj_t p_this);
static	kstatus_t	send(pmsg_obj_t p_this);
static	kstatus_t	recv(pmsg_obj_t p_this);
static	kstatus_t	forward(pmsg_obj_t p_this);
static	kstatus_t	wait(pmsg_obj_t p_this);

pmsg_obj_t	msg_obj(
    u32 major_type,
    u32 minor_type,
    u32 attr,
    pmsg_queue_obj_t p_reply_queue,
    size_t size,
    pheap_t heap)
{
    //Check size
    if(size <= sizeof(msg_obj)) {
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Message size too small.");
        return NULL;
    }

    //Create object
    pmsg_obj_t p_ret = (pmsg_obj_t)obj(
                           CLASS_MSG(major_type),
                           (destructor_t)destructor,
                           (compare_obj_t)compare,
                           (to_string_t)to_string,
                           heap,
                           size);

    if(p_ret == NULL) {
        return NULL;
    }

    //Members
    p_ret->size = size;
    core_pm_mutex_init(&(p_ret->lock));
    core_pm_cond_init(&(p_ret->cond), &(p_ret->lock));
    p_ret->major_type = major_type;
    p_ret->minor_type = minor_type;
    p_ret->status = MSG_STATUS_CREATED;
    p_ret->attr = attr;
    p_ret->reply_queue = p_reply_queue;


    //Methods
    p_ret->complete = complete;
    p_ret->cancel = cancel;
    p_ret->send = send;
    p_ret->recv = recv;
    p_ret->forward = forward;
    p_ret->wait = wait;

    return p_ret;
}

void destructor(pmsg_obj_t p_this)
{
    core_mm_heap_free(p_this, p_this->obj.heap);

    return;
}

int compare(pmsg_obj_t p_this, pmsg_obj_t p_obj)
{
    if(p_this->major_type > p_obj->major_type) {
        return 1;

    } else if(p_this->major_type < p_obj->major_type) {
        return - 1;

    } else {
        return 0;
    }
}

pkstring_obj_t to_string(pmsg_obj_t p_this)
{
    return kstring_fmt("Message object at %p\n"
                       "Major type is 0x%.8X\n"
                       "Minor type is 0x%.8X\n",
                       p_this->obj.heap,
                       p_this,
                       p_this->major_type,
                       p_this->minor_type);
}

void complete(pmsg_obj_t p_this, bool success)
{
    while(core_pm_mutex_acquire(&(p_this->lock), -1) != ESUCCESS);

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

    if(p_this->attr & MSG_ATTR_ASYNC) {
        //Asynchronous message
        //Create complete message
        pmsg_complete_obj_t p_reply_msg;

        if(success) {
            p_reply_msg = msg_complete_obj(
                              p_this,
                              MSG_STATUS_SUCCESS,
                              p_this->obj.heap);

        } else {
            p_reply_msg = msg_complete_obj(
                              p_this,
                              MSG_STATUS_FAILED,
                              p_this->obj.heap);
        }

        //Send complete message
        mstatus_t tmp;
        p_this->reply_queue->send(
            p_this->reply_queue,
            (pmsg_obj_t)p_reply_msg,
            &tmp);
        DEC_REF(p_reply_msg);

    } else {
        //Synchronous message
        core_pm_cond_signal(&(p_this->cond), true);

    }

    return;
}

void cancel(pmsg_obj_t p_this)
{
    while(core_pm_mutex_acquire(&(p_this->lock), -1) != ESUCCESS);

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

    if(p_this->attr & MSG_ATTR_ASYNC) {
        //Asynchronous message
        //Create cancel message
        pmsg_cancel_obj_t p_reply_msg = msg_cancel_obj(
                                            p_this,
                                            p_this->obj.heap);

        //Send cancel message
        mstatus_t tmp;
        p_this->reply_queue->send(
            p_this->reply_queue,
            p_this,
            &tmp);
        DEC_REF(p_reply_msg);

    } else {
        //Synchronous message
        core_pm_cond_signal(&(p_this->cond), true);
    }

    return;
}

kstatus_t send(pmsg_obj_t p_this)
{
    while(core_pm_mutex_acquire(&(p_this->lock), -1) != ESUCCESS);

    //Check status
    if(p_this->status != MSG_STATUS_CREATED) {
        core_pm_mutex_release(&(p_this->lock));
        return EINVAL;
    }

    //Change status
    p_this->status = MSG_STATUS_SENT;

    core_pm_mutex_release(&(p_this->lock));
    return ESUCCESS;
}

kstatus_t recv(pmsg_obj_t p_this)
{
    while(core_pm_mutex_acquire(&(p_this->lock), -1) != ESUCCESS);

    //Check status
    if(p_this->status != MSG_STATUS_SENT) {
        core_pm_mutex_release(&(p_this->lock));
        return EINVAL;
    }

    //Change status
    p_this->status = MSG_STATUS_PENDING;

    core_pm_mutex_release(&(p_this->lock));
    return ESUCCESS;
}

kstatus_t forward(pmsg_obj_t p_this)
{
    while(core_pm_mutex_acquire(&(p_this->lock), -1) != ESUCCESS);

    //Check status
    if(p_this->status != MSG_STATUS_PENDING) {
        core_pm_mutex_release(&(p_this->lock));
        return EINVAL;
    }

    //Change status
    p_this->status = MSG_STATUS_SENT;

    core_pm_mutex_release(&(p_this->lock));
    return ESUCCESS;
}

kstatus_t wait(pmsg_obj_t p_this)
{
    while(core_pm_mutex_acquire(&(p_this->lock), -1) != ESUCCESS);

    if(p_this->status == MSG_STATUS_FAILED
       || p_this->status == MSG_STATUS_SUCCESS
       || p_this->status == MSG_STATUS_CANCELED) {
        core_pm_mutex_release(&(p_this->lock));
        core_exception_set_errno(ESUCCESS);
        return ESUCCESS;

    } else if(p_this->attr & MSG_ATTR_ASYNC) {
        core_pm_mutex_release(&(p_this->lock));
        core_exception_set_errno(ETIMEDOUT);
        return ETIMEDOUT;
    }

    kstatus_t status = core_pm_cond_wait(&(p_this->cond), -1);

    if(status != ESUCCESS) {
        if(p_this->status == MSG_STATUS_FAILED
           || p_this->status == MSG_STATUS_SUCCESS
           || p_this->status == MSG_STATUS_CANCELED) {
            core_exception_set_errno(ESUCCESS);
            return ESUCCESS;
        }

        p_this->attr |= MSG_ATTR_ASYNC;
    }

    core_pm_mutex_release(&(p_this->lock));

    core_exception_set_errno(status);
    return status;
}
