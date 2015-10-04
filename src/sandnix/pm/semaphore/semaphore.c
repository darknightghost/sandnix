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

#include "semaphore.h"
#include "../../exceptions/exceptions.h"
#include "../../debug/debug.h"

void pm_init_semaphore(psemaphore_t p_sem, u32 max_count)
{
	p_sem->count = 0;
	p_sem->max_count = max_count;
	p_sem->blocked_list = NULL;
	pm_init_spn_lock(&(p_sem->lock));
	pm_set_errno(ESUCCESS);
	return;
}

k_status pm_acqr_semaphore(psemaphore_t p_sem, u32 timeout)
{
	plist_node_t p_node;
	sem_wait_thrd_t wait_info;

	pm_acqr_spn_lock(&(p_sem->lock));

	if(p_sem->count < p_sem->max_count && p_sem->blocked_list == NULL) {
		//Get semphore
		(p_sem->count)++;
		pm_rls_spn_lock(&(p_sem->lock));
		pm_set_errno(ESUCCESS);
		return ESUCCESS;

	} else {
		wait_info.awake_flag = false;
		wait_info.thread_id = pm_get_crrnt_thrd_id();
		p_node = rtl_list_insert_after(&(p_sem->blocked_list),
		                               NULL,
		                               &wait_info,
		                               NULL);

		if(p_node == NULL) {

			pm_rls_spn_lock(&(p_sem->lock));
			pm_set_errno(EFAULT);
			return EFAULT;
		}

		//Wait for semaphore
		if(timeout == TIMEOUT_BLOCK) {

			pm_suspend_thrd(pm_get_crrnt_thrd_id());
			pm_rls_spn_lock(&(p_sem->lock));
			pm_schedule();

		} else {

			pm_sleep(timeout);
			pm_rls_spn_lock(&(p_sem->lock));
			pm_schedule();
		}
	}

	//Try again
	pm_acqr_spn_lock(&(p_sem->lock));
	rtl_list_remove(&(p_sem->blocked_list), p_node, NULL);

	if(p_sem->count < p_sem->max_count && wait_info.awake_flag) {
		//Get semphore
		(p_sem->count)++;
		pm_rls_spn_lock(&(p_sem->lock));
		pm_set_errno(ESUCCESS);
		return ESUCCESS;

	}

	pm_rls_spn_lock(&(p_sem->lock));

	if(pm_should_break()) {
		pm_set_errno(EINVAL);
		return EINVAL;

	} else {
		pm_set_errno(EAGAIN);
		return EAGAIN;
	}
}

k_status pm_try_acqr_semaphore(psemaphore_t p_sem)
{
	pm_acqr_spn_lock(&(p_sem->lock));

	if(p_sem->count < p_sem->max_count && p_sem->blocked_list == NULL) {
		//Get semphore
		(p_sem->count)++;
		pm_rls_spn_lock(&(p_sem->lock));
		pm_set_errno(ESUCCESS);
		return ESUCCESS;

	}

	pm_rls_spn_lock(&(p_sem->lock));
	pm_set_errno(EAGAIN);
	return EAGAIN;
}

void pm_rls_semaphore(psemaphore_t p_sem)
{
	psem_wait_thrd_t p_wait;
	plist_node_t p_node;

	pm_acqr_spn_lock(&(p_sem->lock));

	if(p_sem->count > 0) {
		//Decrease count
		(p_sem->count)--;

		//Awake thread
		if(p_sem->blocked_list != NULL) {
			p_node = p_sem->blocked_list;

			do {
				p_wait = (psem_wait_thrd_t)(p_node->p_item);

				if(!(p_wait->awake_flag)) {
					p_wait->awake_flag = true;
					pm_resume_thrd(p_wait->thread_id);
					break;
				}

				p_node = p_node->p_next;
			} while(p_node != p_sem->blocked_list);
		}

		pm_rls_spn_lock(&(p_sem->lock));
		pm_set_errno(ESUCCESS);
		return;

	} else {
		pm_rls_spn_lock(&(p_sem->lock));
		pm_set_errno(EINVAL);
		return;
	}
}
