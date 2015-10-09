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

#include "ssddt.h"
#include "ssddt_funcs.h"
#include "../../rtl/rtl.h"
#include "../../vfs/vfs.h"
#include "../../msg/msg.h"
#include "../../mm/mm.h"
#include "../../pm/pm.h"
#include "../../exceptions/exceptions.h"

typedef	struct {
	kobject_t	obj;
	u32			index;
	semaphore_t	semaphore;
} sem_obj_t, *psem_obj_t;

static	char*	sem_obj_name = "semaphore_obj";

static	void	sem_obj_destroyer(pkobject_t p_obj);

void* ssddt_create_semaphore(va_list p_args)
{
	//Agruments
	u32 max_count;

	//Variables
	k_status status;
	u32 index;

	//Get args
	max_count = va_arg(p_args, u32);

	p_obj = mm_hp_alloc(sizeof(sem_obj_t), NULL);

	if(p_obj == NULL) {
		pm_set_errno(EFAULT);
		return;
	}

	vfs_initialize_object((pkobject_t)p_obj);
	p_obj->obj.size = sizeof(sem_obj_t);
	p_obj->obj.name = sem_obj_name;
	pm_init_semaphore(&(p_obj->semaphore), max_count);
	p_obj->obj.destroy_callback = sem_obj_destroyer;

	index = vfs_add_proc_obj(p_obj);
	status = pm_get_errno();

	if(status != ESUCCESS) {
		mm_hp_free(p_obj, NULL);
		pm_set_errno(status);
		return NULL;
	}

	vfs_dec_obj_reference((pkobject_t)p_obj);
	p_obj->index = index;

	pm_set_errno(status);
	return p_obj;
}

k_status ssddt_acqr_semaphore(va_list p_args)
{
	//Agruments
	void* p_sem_obj;
	u32 timeout;

	//Variables

	//Get args
	p_sem_obj = va_arg(p_args, psem_obj_t);
	timeout = va_arg(p_args, u32);

	//Check arguments
	if(!mm_virt_test(p_sem_obj, sizeof(sem_obj_t), PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	if(p_sem_obj->obj.name != sem_obj_name) {
		pm_set_errno(EINVAL);
		return;
	}

	pm_acqr_semaphore(&(p_obj->semaphore), timeout);

	return pm_get_errno();
}

k_status ssddt_try_semaphore(va_list p_args)
{
	//Agruments
	void* p_sem_obj;

	//Variables

	//Get args
	p_sem_obj = va_arg(p_args, psem_obj_t);

	//Check arguments
	if(!mm_virt_test(p_sem_obj, sizeof(sem_obj_t), PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	if(p_sem_obj->obj.name != sem_obj_name) {
		pm_set_errno(EINVAL);
		return;
	}

	pm_try_acqr_semaphore(&(p_obj->semaphore));

	return pm_get_errno();
}

void ssddt_rls_semaphore(va_list p_args)
{
	//Agruments
	void* p_sem_obj;

	//Variables

	//Get args
	p_sem_obj = va_arg(p_args, psem_obj_t);

	//Check arguments
	if(!mm_virt_test(p_sem_obj, sizeof(sem_obj_t), PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	if(p_sem_obj->obj.name != sem_obj_name) {
		pm_set_errno(EINVAL);
		return;
	}

	pm_rls_semaphore(&(p_obj->semaphore));

	return pm_get_errno();
}

void ssddt_destroy_semaphore(va_list p_args)
{
	//Agruments
	void* p_sem_obj;

	//Variables

	//Get args
	p_sem_obj = va_arg(p_args, psem_obj_t);

	//Check arguments
	if(!mm_virt_test(p_sem_obj, sizeof(sem_obj_t), PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	if(p_sem_obj->obj.name != sem_obj_name) {
		pm_set_errno(EINVAL);
		return;
	}

	vfs_remove_proc_obj(p_sem_obj->index);

	return;
}

void sem_obj_destroyer(pkobject_t p_obj)
{
	mm_hp_free(p_obj, NULL);
	return;
}
