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

#include "../io.h"
#include "../../vfs/vfs.h"
#include "../../rtl/rtl.h"
#include "../../pm/pm.h"
#include "../../msg/msg.h"
#include "../../debug/debug.h"
#include "../../exceptions/exceptions.h"
#include "int_msg.h"

static	list_t			int_drvs[INTERRUPT_MAX_NUM];
static	spinlock_t		int_drvs_lock;
static	int_msg_info_t	int_info[INTERRUPT_MAX_NUM];
static	u32				send_thread_id;
static	bool			restart_flag;

static	void		inc_int_info_ref(u32 int_num);
static	void		dec_int_info_ref(u32 int_num);
static	void		send_thread(u32 id, void* p_null);
static	bool		int_hndlr_func(u32 int_num, u32 thread_id, u32 err_code);
static	void		send_msg(u32 driver_id, u32 int_num, u32 count);

void io_int_msg_init()
{
	u32 i;
	rtl_memset(int_drvs, 0, sizeof(int_drvs));

	for(i = 0; i < INTERRUPT_MAX_NUM; i++) {
		int_info[i].ref_count = 0;
		int_info[i].count = 0;
		pm_init_spn_lock(&(int_info[i].count_lock));
		int_info[i].info.func = int_hndlr_func;
		int_info[i].info.p_next = NULL;
	}

	pm_init_spn_lock(&int_drvs_lock);
	restart_flag = false;

	send_thread_id = pm_create_thrd(send_thread,
	                                true,
	                                false,
	                                INT_LEVEL_DISPATCH,
	                                NULL);
	ASSERT(OPERATE_SUCCESS);
	return;
}

bool io_int_msg_set(u32 int_num)
{
	u32 driver_id;
	plist_node_t p_node;

	if(int_num >= INTERRUPT_MAX_NUM) {
		return false;
	}

	if(IS_INT_RESERVED(int_num)) {
		return false;
	}

	driver_id = vfs_get_crrnt_driver_id();

	//If the driver id has been add
	pm_acqr_spn_lock(&int_drvs_lock);
	p_node = int_drvs[int_num];

	if(p_node != NULL) {
		do {
			if(((u32)(p_node->p_item)) == driver_id) {
				pm_rls_spn_lock(&int_drvs_lock);
				return true;
			}

			p_node = p_node->p_next;
		} while(p_node != int_drvs[driver_id]);
	}

	rtl_list_insert_after(&int_drvs[int_num], NULL, (void*)driver_id, NULL);
	inc_int_info_ref(int_num);
	pm_rls_spn_lock(&int_drvs_lock);
	return true;
}

void io_int_msg_clean(u32 int_num)
{
	u32 driver_id;
	plist_node_t p_node, p_remove_node;
	u32 i;

	if(int_num >= INTERRUPT_MAX_NUM && int_num != ALL_INTERRUPT) {
		return;
	}

	driver_id = vfs_get_crrnt_driver_id();

	pm_acqr_spn_lock(&int_drvs_lock);

	if(int_num == ALL_INTERRUPT) {
		for(i = 0; i < INTERRUPT_MAX_NUM; i++) {
			p_node = int_drvs[i];

			if(p_node != NULL) {
				do {
					if(((u32)(p_node->p_item)) == driver_id) {
						p_remove_node = p_node;
						p_node = p_node->p_next;
						rtl_list_remove(&int_drvs[i], p_remove_node, NULL);
						dec_int_info_ref(i);
						break;

					} else {
						p_node = p_node->p_next;
					}
				} while(p_node != int_drvs[driver_id]);
			}

		}

	} else {
		p_node = int_drvs[int_num];

		if(p_node != NULL) {
			do {
				if(((u32)(p_node->p_item)) == driver_id) {
					p_remove_node = p_node;
					p_node = p_node->p_next;
					rtl_list_remove(&int_drvs[int_num], p_remove_node, NULL);
					dec_int_info_ref(int_num);
					break;

				} else {
					p_node = p_node->p_next;
				}
			} while(p_node != int_drvs[driver_id]);
		}

	}

	pm_rls_spn_lock(&int_drvs_lock);
	return;
}

void inc_int_info_ref(u32 int_num)
{
	if(int_info[int_num].ref_count == 0) {
		io_reg_int_hndlr(int_num, &(int_info[int_num].info));
	}

	(int_info[int_num].ref_count)++;

	return;
}

void dec_int_info_ref(u32 int_num)
{
	(int_info[int_num].ref_count)--;

	if(int_info[int_num].ref_count == 0) {
		io_unreg_int_hndlr(int_num, &(int_info[int_num].info));
	}

	return;
}

void send_thread(u32 id, void* p_null)
{
	u32 i;
	u32 count;
	plist_node_t p_node;

	while(1) {
		pm_suspend_thrd(id);

		for(i = 0; i < INTERRUPT_MAX_NUM; i++) {
			if(int_info[i].ref_count != 0) {
				pm_acqr_spn_lock(&(int_info[i].count_lock));
				count = int_info[i].count;
				int_info[i].count = 0;
				pm_rls_spn_lock(&(int_info[i].count_lock));

				//Send messages
				pm_acqr_spn_lock(&int_drvs_lock);
				p_node = int_drvs[i];

				do {
					send_msg((u32)(p_node->p_item), i, count);
					p_node = p_node->p_next;
				} while(p_node != int_drvs[i]);

				pm_rls_spn_lock(&int_drvs_lock);
			}

			if(restart_flag) {
				restart_flag = false;
				i = 0;
			}
		}
	}

	UNREFERRED_PARAMETER(p_null);
}

bool int_hndlr_func(u32 int_num, u32 thread_id, u32 err_code)
{
	if(int_info[int_num].ref_count != 0) {
		pm_acqr_spn_lock(&(int_info[int_num].count_lock));
		(int_info[int_num].count)++;
		restart_flag = true;
		pm_rls_spn_lock(&(int_info[int_num].count_lock));
	}

	pm_resume_thrd(send_thread_id);
	UNREFERRED_PARAMETER(thread_id);
	UNREFERRED_PARAMETER(err_code);
	return false;
}

void send_msg(u32 driver_id, u32 int_num, u32 count)
{
	u32 i;
	pmsg_t p_msg;
	pmsg_interrupt_info_t p_info;
	u32 result;
	k_status complete_result;

	for(i = 0; i < count; i++) {
		msg_create(&p_msg,
		           sizeof(msg_t) + sizeof(msg_interrupt_info_t));
		ASSERT(OPERATE_SUCCESS);
		p_info = (pmsg_interrupt_info_t)(p_msg + 1);
		p_msg->buf.addr = p_info;
		p_msg->flags.flags = MFLAG_DIRECTBUF | MFLAG_ASYNC;
		p_msg->message = MSG_INTERRUPT;
		p_info->int_num = int_num;
		vfs_send_drv_message(vfs_get_crrnt_driver_id(),
		                     driver_id,
		                     p_msg,
		                     &result,
		                     &complete_result);
	}

	return;
}
