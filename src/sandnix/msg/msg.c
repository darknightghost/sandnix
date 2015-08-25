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

#include "msg.h"
#include "../pm/pm.h"
#include "../mm/mm.h"
#include "../exceptions/exceptions.h"
#include "../debug/debug.h"

static	pmsg_queue_t	msg_queue_table[MAX_MSG_QUEUE_NUM];
static	mutex_t			msg_queue_table_lock;
static	u32				current_id;
static	spinlock_t		id_lock;


void msg_init()
{
	dbg_print("\nInitializing Message Manager module...\n");

	rtl_memset(msg_queue_table, 0, sizeof(msg_queue_table));
	pm_init_mutex(&msg_queue_table_lock);
	current_id = 0;
	pm_init_spn_lock(&id_lock);

	return;
}

u32 msg_queue_create()
{
	u32 new_id;

	pm_acqr_mutex(&msg_queue_table_lock, TIMEOUT_BLOCK);

	//Get new id
	for(new_id = 0; new_id < MAX_MSG_QUEUE_NUM; new_id++) {
		if(msg_queue_table[new_id] == NULL) {
			//Allocate new message queue_t
			msg_queue_table[new_id] = mm_hp_alloc(sizeof(msg_queue_t), NULL);

			if(msg_queue_table[new_id] == NULL) {

				pm_rls_mutex(&msg_queue_table_lock);

				pm_set_errno(EFAULT);

				return 0;
			}

			rtl_queue_init(&(msg_queue_table[new_id]->msgs));
			pm_init_mutex(&(msg_queue_table[new_id]->lock));
			msg_queue_table[new_id]->blocked_thread_id = 0;
			msg_queue_table[new_id]->destroy_flag = false;

			pm_rls_mutex(&msg_queue_table_lock);

			pm_set_errno(ESUCCESS);

			return new_id;
		}
	}

	pm_rls_mutex(&msg_queue_table_lock);

	pm_set_errno(EAGAIN);

	return 0;
}

void msg_queue_destroy(u32 id)
{
	pmsg_queue_t p_queue;
	pmsg_t p_msg;

	pm_acqr_mutex(&msg_queue_table_lock, TIMEOUT_BLOCK);

	//Refresh id
	if(msg_queue_table[id] == NULL) {
		pm_rls_mutex(&msg_queue_table_lock);
		pm_set_errno(EFAULT);
		return;
	}

	p_queue = msg_queue_table[id];
	msg_queue_table[id] = NULL;
	pm_rls_mutex(&msg_queue_table_lock);

	//Destroy the queue_t
	if(p_queue->blocked_thread_id == 0) {
		p_queue->destroy_flag = true;
		pm_resume_thrd(p_queue->blocked_thread_id);
	}

	for(p_msg = rtl_queue_pop(&(p_queue->msgs), NULL);
	    p_msg != NULL;
	    p_msg = rtl_queue_pop(&(p_queue->msgs), NULL)) {
		msg_cancel(p_msg);
	}

	//Wait for the thread
	while(p_queue->blocked_thread_id != 0) {
		pm_schedule();
	}

	pm_set_errno(ESUCCESS);

	return;
}

k_status msg_create(pmsg_t* p_p_msg, size_t size)
{
	*p_p_msg = mm_hp_alloc(size, NULL);

	if(*p_p_msg == NULL) {
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	rtl_memset(*p_p_msg, 0, size);

	//status
	(*p_p_msg)->status = MSTATUS_FORWARD;
	(*p_p_msg)->size = size;

	//msg_id
	pm_acqr_spn_lock(&id_lock);
	(*p_p_msg)->msg_id = current_id;
	current_id++;
	pm_rls_spn_lock(&id_lock);

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

k_status msg_send(pmsg_t p_msg, u32 dest_queue, u32* p_result)
{
	pm_acqr_mutex(&msg_queue_table_lock, TIMEOUT_BLOCK);

	//Check if the queue_t exists
	if(msg_queue_table[dest_queue] == NULL) {
		pm_rls_mutex(&msg_queue_table_lock);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	//src_thread
	p_msg->src_thread = pm_get_crrnt_thrd_id();

	pm_acqr_mutex(&(msg_queue_table[dest_queue]->lock), TIMEOUT_BLOCK);

	//Add message
	if(!rtl_queue_push(&(msg_queue_table[dest_queue]->msgs),
	                   p_msg,
	                   NULL)) {
		pm_rls_mutex(&(msg_queue_table[dest_queue]->lock));
		pm_rls_mutex(&msg_queue_table_lock);

		pm_set_errno(EFAULT);
		return EFAULT;
	}

	if(p_msg->flags.flags | MFLAG_ASYNC) {
		pm_rls_mutex(&(msg_queue_table[dest_queue]->lock));
		pm_rls_mutex(&msg_queue_table_lock);

	} else {
		pm_disable_task_switch();

		//Wait for result
		pm_suspend_thrd(pm_get_crrnt_thrd_id());
		pm_rls_mutex(&(msg_queue_table[dest_queue]->lock));
		pm_rls_mutex(&msg_queue_table_lock);

		pm_enable_task_switch();
		pm_schedule();

		*p_result = p_msg->status;
		mm_hp_free(p_msg, NULL);

	}

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

k_status msg_recv(pmsg_t* p_p_msg, u32 dest_queue, bool if_block)
{
	pmsg_queue_t p_queue;

	pm_acqr_mutex(&msg_queue_table_lock, TIMEOUT_BLOCK);

	//Check if the queue_t exists
	if(msg_queue_table[dest_queue] == NULL) {
		pm_rls_mutex(&msg_queue_table_lock);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	p_queue = msg_queue_table[dest_queue];

	if(p_queue->blocked_thread_id != 0) {
		pm_rls_mutex(&msg_queue_table_lock);
		pm_set_errno(EDEADLK);
		return EDEADLK;
	}

	pm_acqr_mutex(&(p_queue->lock), TIMEOUT_BLOCK);

	*p_p_msg = rtl_queue_front(&(p_queue->msgs));

	if(*p_p_msg == NULL) {
		if(if_block) {
			//Wait for new message
			p_queue->blocked_thread_id = pm_get_crrnt_thrd_id();
			pm_disable_task_switch();
			pm_suspend_thrd(p_queue->blocked_thread_id);
			pm_rls_mutex(&(p_queue->lock));
			pm_rls_mutex(&msg_queue_table_lock);
			pm_enable_task_switch();
			pm_schedule();

			//Check if the queue_t is being destroyed
			pm_acqr_mutex(&(p_queue->lock), TIMEOUT_BLOCK);

			if(p_queue->destroy_flag) {
				pm_rls_mutex(&(p_queue->lock));
				p_queue->blocked_thread_id = 0;
				pm_set_errno(EINTR);
				return EINTR;
			}

		} else {
			pm_rls_mutex(&(p_queue->lock));
			pm_rls_mutex(&msg_queue_table_lock);

			pm_set_errno(EAGAIN);
			return EAGAIN;
		}

	} else {
		pm_rls_mutex(&msg_queue_table_lock);
	}

	*p_p_msg = rtl_queue_pop(&(p_queue->msgs), NULL);

	if(*p_p_msg == NULL) {
		pm_rls_mutex(&(p_queue->lock));

		pm_set_errno(EFAULT);
		return EFAULT;
	}

	pm_rls_mutex(&(p_queue->lock));

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

k_status msg_forward(pmsg_t p_msg, u32 dest_queue)
{
	p_msg->status = MSTATUS_FORWARD;

	pm_acqr_mutex(&msg_queue_table_lock, TIMEOUT_BLOCK);

	//Check if the queue_t exists
	if(msg_queue_table[dest_queue] == NULL) {
		pm_rls_mutex(&msg_queue_table_lock);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	pm_acqr_mutex(&(msg_queue_table[dest_queue]->lock), TIMEOUT_BLOCK);

	//Add message
	if(!rtl_queue_push(&(msg_queue_table[dest_queue]->msgs),
	                   p_msg,
	                   NULL)) {
		pm_rls_mutex(&(msg_queue_table[dest_queue]->lock));
		pm_rls_mutex(&msg_queue_table_lock);

		pm_set_errno(EFAULT);
		return EFAULT;
	}

	pm_rls_mutex(&(msg_queue_table[dest_queue]->lock));
	pm_rls_mutex(&msg_queue_table_lock);

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

k_status msg_complete(pmsg_t p_msg)
{
	pmsg_t p_complete_msg;
	k_status status;
	pmsg_complete_info_t p_complete_info;

	if(p_msg->flags.flags | MFLAG_ASYNC) {

		if(p_msg->message == MSG_COMPLETE
		   || p_msg->message == MSG_CANCEL) {
			mm_hp_free(p_msg->buf.buf.addr, NULL);
			mm_hp_free(p_msg, NULL);

			pm_set_errno(ESUCCESS);
			return ESUCCESS;

		} else {
			//Send complete message
			status = msg_create(&p_complete_msg, sizeof(msg_t));

			if(status != ESUCCESS) {
				return status;
			}

			p_complete_msg->message = MSG_COMPLETE;
			p_complete_msg->flags.flags = MFLAG_DIRECTBUF | MFLAG_ASYNC;

			p_complete_info = mm_hp_alloc(sizeof(msg_complete_info_t), NULL);

			if(p_complete_info == NULL) {
				mm_hp_free(p_complete_msg, NULL);
				pm_set_errno(EFAULT);
				return EFAULT;
			}

			p_complete_info->msg_id = p_msg->msg_id;

			p_complete_msg->buf.buf.addr = p_complete_info;
			p_complete_msg->buf.buf.size = sizeof(msg_complete_info_t);

			status = msg_send(p_complete_msg, p_msg->result_queue, NULL);

			return status;
		}

	} else {
		//Set status
		p_msg->status = MSTATUS_COMPLETE;

		//Awake thread
		pm_resume_thrd(p_msg->src_thread);

		pm_set_errno(ESUCCESS);
		return ESUCCESS;
	}
}

k_status	msg_cancel(pmsg_t p_msg)
{
	pmsg_t p_cancel_msg;
	k_status status;
	pmsg_cancel_info_t p_cancel_info;

	if(p_msg->flags.flags | MFLAG_ASYNC) {

		if(p_msg->message == MSG_COMPLETE
		   || p_msg->message == MSG_CANCEL) {
			mm_hp_free(p_msg->buf.buf.addr, NULL);
			mm_hp_free(p_msg, NULL);

			pm_set_errno(ESUCCESS);
			return ESUCCESS;

		} else {
			//Send cancel message
			status = msg_create(&p_cancel_msg, sizeof(msg_t));

			if(status != ESUCCESS) {
				return status;
			}

			p_cancel_msg->message = MSG_CANCEL;
			p_cancel_msg->flags.flags = MFLAG_DIRECTBUF | MFLAG_ASYNC;

			p_cancel_info = mm_hp_alloc(sizeof(msg_cancel_info_t), NULL);

			if(p_cancel_info == NULL) {
				mm_hp_free(p_cancel_msg, NULL);
				pm_set_errno(EFAULT);
				return EFAULT;
			}

			p_cancel_info->msg_id = p_msg->msg_id;

			p_cancel_msg->buf.buf.addr = p_cancel_info;
			p_cancel_msg->buf.buf.size = sizeof(msg_cancel_info_t);

			status = msg_send(p_cancel_msg, p_msg->result_queue, NULL);

			return status;
		}

	} else {
		//Set status
		p_msg->status = MSTATUS_CANCEL;

		//Awake thread
		pm_resume_thrd(p_msg->src_thread);

		pm_set_errno(ESUCCESS);
		return ESUCCESS;
	}
}
