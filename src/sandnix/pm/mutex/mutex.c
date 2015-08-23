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

#include "../pm.h"
#include "../../io/io.h"

void pm_init_mutex(pmutex_t p_mutex)
{
	pm_init_spn_lock(&(p_mutex->lock));
	p_mutex->is_acquired = false;
	p_mutex->acquire_list = NULL;
	p_mutex->next_thread = (0 - 1);
	return;
}

k_status  pm_acqr_mutex(pmutex_t p_mutex, u32 timeout)
{
	plist_node_t p_node;
	plist_node_t p_current_node;
	u32 current_priority;

	current_priority = io_get_crrnt_int_level();

	pm_acqr_spn_lock(&(p_mutex->lock));
	p_node = p_mutex->acquire_list;

	if(p_node != NULL || p_mutex->is_acquired) {
		//If other thread has acquired the mutex_t
		//Add current thread to the list_t
		if(p_node != NULL) {
			if(pm_get_thread_priority((u32)(p_node->p_item)) < current_priority) {
				p_node = NULL;

			} else {
				do {
					if(pm_get_thread_priority((u32)(p_node->p_item)) < current_priority) {
						break;
					}

					p_node = p_node->p_next;
				} while(p_node != p_mutex->acquire_list);

			}
		}

		p_current_node = rtl_list_insert_before(&(p_mutex->acquire_list),
		                                        p_node,
		                                        (void*)pm_get_crrnt_thrd_id(),
		                                        NULL);

		if(p_current_node == NULL) {
			pm_rls_spn_lock(&(p_mutex->lock));
			pm_set_errno(EAGAIN);
			return EAGAIN;
		}

		//Wait for lock
		pm_rls_spn_lock(&(p_mutex->lock));

		if(timeout == TIMEOUT_BLOCK) {
			pm_suspend_thrd(pm_get_crrnt_thrd_id());

		} else {
			pm_sleep(timeout);
		}

		pm_acqr_spn_lock(&(p_mutex->lock));

		rtl_list_remove(&(p_mutex->acquire_list), p_current_node, NULL);

		if(p_mutex->next_thread == pm_get_crrnt_thrd_id()) {
			p_mutex->is_acquired = true;
			pm_rls_spn_lock(&(p_mutex->lock));
			pm_set_errno(ESUCCESS);
			return ESUCCESS;

		} else {
			pm_rls_spn_lock(&(p_mutex->lock));
			pm_set_errno(EINTR);
			return EINTR;
		}


	} else {
		//Acquired the mutex_t
		p_mutex->is_acquired = true;
		pm_rls_spn_lock(&(p_mutex->lock));
		pm_set_errno(ESUCCESS);
		return ESUCCESS;
	}
}

k_status pm_try_acqr_mutex(pmutex_t p_mutex)
{
	pm_acqr_spn_lock(&(p_mutex->lock));

	if(p_mutex->is_acquired == false
	   && p_mutex->acquire_list == NULL) {
		p_mutex->is_acquired = true;
		pm_rls_spn_lock(&(p_mutex->lock));
		pm_set_errno(ESUCCESS);
		return ESUCCESS;

	} else {
		pm_rls_spn_lock(&(p_mutex->lock));
		pm_set_errno(EAGAIN);
		return EAGAIN;
	}

}

void pm_rls_mutex(pmutex_t p_mutex)
{
	plist_node_t p_node;

	pm_acqr_spn_lock(&(p_mutex->lock));

	//Release the mutex
	p_mutex->is_acquired = false;
	p_node = p_mutex->acquire_list;

	if(p_node == NULL) {
		pm_rls_spn_lock(&(p_mutex->lock));
		pm_set_errno(ESUCCESS);
		return;
	}

	//Awake next thread
	p_mutex->next_thread = (u32)(p_node->p_item);
	pm_resume_thrd((u32)(p_node->p_item));
	pm_rls_spn_lock(&(p_mutex->lock));

	pm_set_errno(ESUCCESS);
	return;
}
