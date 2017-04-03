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

#include "../../../../../common/common.h"
#include "../../pm/pm.h"
#include "../../mm/mm.h"
#include "../../exception/exception.h"

#include "./msg_queue_obj.h"

static	void			destructor(pmsg_queue_obj_t p_this);
static	int				compare(pmsg_queue_obj_t p_this, pmsg_queue_obj_t p_obj);
static	pkstring_obj_t	to_string(pmsg_queue_obj_t p_this);

static	kstatus_t		send(pmsg_queue_obj_t p_this, pmsg_obj_t p_msg,
                             mstatus_t* result);
static	kstatus_t		recv(pmsg_queue_obj_t p_this, pmsg_obj_t* p_ret,
                             s32 millisec_timeout);
static	u32				get_msg_count(pmsg_queue_obj_t p_this);
static	void			destroy(pmsg_queue_obj_t p_this);

pmsg_queue_obj_t msg_queue(pheap_t heap)
{
    //Create object
    pmsg_queue_obj_t p_ret = (pmsg_queue_obj_t)obj(
                                 CLASS_MSG_QUEUE_OBJECT,
                                 (destructor_t)destructor,
                                 (compare_obj_t)compare,
                                 (to_string_t)to_string,
                                 heap,
                                 sizeof(msg_queue_obj_t));

    if(p_ret == NULL) {
        return NULL;
    }

    //Member variables
    core_rtl_queue_init(&(p_ret->msg_queue), heap);
    p_ret->msg_count = 0;
    core_pm_mutex_init(&(p_ret->lock));
    core_pm_cond_init(&(p_ret->msg_cond), &(p_ret->lock));
    p_ret->alive = true;

    //Methods
    p_ret->send = send;
    p_ret->recv = recv;
    p_ret->get_msg_count = get_msg_count;
    p_ret->destroy = destroy;

    return p_ret;
}

void destructor(pmsg_queue_obj_t p_this)
{
    if(p_this->alive) {
        p_this->destroy(p_this);
    }

    core_mm_heap_free(p_this, p_this->obj.heap);

    return;
}

int compare(pmsg_queue_obj_t p_this, pmsg_queue_obj_t p_obj)
{
    if((address_t)p_this > (address_t)p_obj) {
        return 1;

    } else if((address_t)p_this < (address_t)p_obj) {
        return -1;

    } else {
        return 0;
    }
}

pkstring_obj_t to_string(pmsg_queue_obj_t p_this)
{
    return kstring_fmt(
               "Message queue at %p\n."
               "%u messages in the queue.\n",
               p_this->obj.heap,
               p_this,
               p_this->msg_count);
}

kstatus_t send(pmsg_queue_obj_t p_this, pmsg_obj_t p_msg,
               mstatus_t* result)
{
    while(core_pm_mutex_acquire(&(p_this->lock), -1) != ESUCCESS);

    //Check queue status
    if(!p_this->alive) {
        core_pm_mutex_release(&(p_this->lock));
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Dead message queue.");
        return EINVAL;
    }

    kstatus_t status = p_msg->send(p_msg);

    if(status != ESUCCESS) {
        core_pm_mutex_release(&(p_this->lock));
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Illegal message.");
        return EINVAL;
    }

    //Push message
    core_rtl_queue_push(&(p_this->msg_queue), p_msg);
    (p_this->msg_count)++;
    INC_REF(p_msg);

    //Set cond
    core_pm_cond_signal(&(p_this->msg_cond), false);

    core_pm_mutex_release(&(p_this->lock));

    //Wait for message
    status = p_msg->wait(p_msg);

    if(status == ESUCCESS) {
        *result = p_msg->status;
    }

    return status;

}

kstatus_t recv(pmsg_queue_obj_t p_this, pmsg_obj_t* p_ret,
               s32 millisec_timeout)
{
    kstatus_t status = ESUCCESS;

    while(core_pm_mutex_acquire(&(p_this->lock), -1) != ESUCCESS);

    //Check queue status
    if(!p_this->alive) {
        core_pm_mutex_release(&(p_this->lock));
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Dead message queue.");
        return EINVAL;
    }

    while(p_this->msg_count == 0) {
        //Wait for message
        status = core_pm_cond_wait(&(p_this->msg_cond), millisec_timeout);

        if(status != ESUCCESS) {
            core_pm_mutex_release(&(p_this->lock));
            core_exception_set_errno(status);
            return status;
        }

        if(!p_this->alive) {
            core_pm_mutex_release(&(p_this->lock));
            core_exception_set_errno(EOWNERDEAD);
            return EOWNERDEAD;
        }
    }

    //Pop message
    (p_this->msg_count)--;
    pmsg_obj_t p_msg = core_rtl_queue_pop(&(p_this->msg_queue));
    core_pm_mutex_release(&(p_this->lock));

    p_msg->recv(p_msg);
    *p_ret = p_msg;

    core_exception_set_errno(ESUCCESS);
    return ESUCCESS;
}

u32 get_msg_count(pmsg_queue_obj_t p_this)
{
    while(core_pm_mutex_acquire(&(p_this->lock), -1) != ESUCCESS);

    u32 ret = p_this->msg_count;

    core_pm_mutex_release(&(p_this->lock));

    return ret;
}

void destroy(pmsg_queue_obj_t p_this)
{
    while(core_pm_mutex_acquire(&(p_this->lock), -1) != ESUCCESS);

    //Check queue status
    if(!p_this->alive) {
        core_pm_mutex_release(&(p_this->lock));
        peinval_except_t p_except = einval_except();
        RAISE(p_except, "Dead message queue.");
        return;
    }

    //Set status
    p_this->alive = false;
    core_pm_cond_signal(&(p_this->msg_cond),
                        true);
    core_pm_mutex_release(&(p_this->lock));

    //Cancel all messages
    while(p_this->msg_count > 0) {
        (p_this->msg_count)--;
        pmsg_obj_t p_msg = core_rtl_queue_pop(&(p_this->msg_queue));
        p_msg->cancel(p_msg);
        DEC_REF(p_msg);
    }

    return;
}
