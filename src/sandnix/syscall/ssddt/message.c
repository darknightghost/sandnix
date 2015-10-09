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
	pmsg_t		p_msg;
	mutex_t		lock;
} msg_obj_t, *pmsg_obj_t;

static	char*	msg_obj_name = "msg_obj";

static	void	msg_obj_destroyer(pkobject_t p_obj);

void* ssddt_recv_msg(va_list p_args)
{
	//Agruments
	size_t* p_msg_len;
	bool if_block;

	//Variables
	pmsg_t p_msg;
	k_status status;
	u32 index;
	pmsg_obj_t p_obj;

	//Get args
	p_msg_len = va_arg(p_args, size_t*);
	if_block = va_arg(p_args, bool);

	//Check arguments
	if(!mm_virt_test(p_msg_len, sizeof(size_t*),
	                 PG_STAT_COMMIT | PG_STAT_WRITEABLE, true)) {
		pm_set_errno(EINVAL);
		return NULL;
	}

	status = vfs_recv_drv_message(vfs_get_crrnt_driver_id(),
	                              &p_msg, if_block);

	if(statu != ESUCCESS) {
		return NULL;
	}

	p_obj = mm_hp_alloc(sizeof(msg_obj_t), NULL);

	if(p_obj == NULL) {
		pm_set_errno(EFAULT);
		return;
	}

	vfs_initialize_object((pkobject_t)p_obj);
	p_obj->obj.size = sizeof(msg_obj_t);
	p_obj->obj.name = msg_obj_name;
	p_obj->p_msg = p_msg;
	pm_init_mutex(&(p_obj->lock));
	p_obj->obj.destroy_callback = msg_obj_destroyer;

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

k_status ssddt_complete_msg(va_list p_args)
{
	//Agruments
	pmsg_obj_t p_msg_obj;
	k_status msg_status;

	//Variables
	k_status status;

	//Get args
	p_msg_obj = va_arg(p_args, void*);
	msg_status = va_arg(p_args, k_status);

	//Check arguments
	if(!mm_virt_test(p_msg_obj, sizeof(msg_obj_t), PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	if(p_msg_obj->obj.name != msg_obj_name) {
		pm_set_errno(EINVAL);
		return;
	}

	if(p_msg_obj->p_msg == NULL) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	pm_acqr_mutex(&(p_msg_obj->lock), TIMEOUT_BLOCK);
	status = msg_complete(p_msg_obj->p_msg, msg_status);

	if(status == ESUCCESS) {
		p_msg_obj->p_msg = NULL;
	}

	pm_rls_mutex(&(p_msg_obj->lock));

	if(status == ESUCCESS) {
		vfs_dec_obj_reference((pkobject_t)p_msg_obj);
	}

	pm_set_errno(status);
	return status;
}

k_status ssddt_forward_msg(va_list p_args)
{
	//Agruments
	pmsg_obj_t p_msg_obj;
	u32 dev_num;

	//Variables

	//Get args
	p_msg_obj = va_arg(p_args, void*);
	dev_num = va_arg(p_args, u32);

	//Check arguments
	if(!mm_virt_test(p_msg_obj, sizeof(msg_obj_t), PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	if(p_msg_obj->obj.name != msg_obj_name) {
		pm_set_errno(EINVAL);
		return;
	}

	if(p_msg_obj->p_msg == NULL) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	pm_acqr_mutex(&(p_msg_obj->lock), TIMEOUT_BLOCK);
	status = vfs_msg_forward(p_msg_obj->p_msg, dev_num);

	if(status == ESUCCESS) {
		p_msg_obj->p_msg = NULL;
	}

	pm_rls_mutex(&(p_msg_obj->lock));

	if(status == ESUCCESS) {
		vfs_dec_obj_reference((pkobject_t)p_msg_obj);
	}

	pm_set_errno(status);
	return status;
}

void ssddt_cancel_msg(va_list p_args)
{
	//Agruments
	pmsg_obj_t p_msg_obj;

	//Variables
	k_status status;

	//Get args
	p_msg_obj = va_arg(p_args, void*);

	//Check arguments
	if(!mm_virt_test(p_msg_obj, sizeof(msg_obj_t), PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	if(p_msg_obj->obj.name != msg_obj_name) {
		pm_set_errno(EINVAL);
		return;
	}

	if(p_msg_obj->p_msg == NULL) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	pm_acqr_mutex(&(p_msg_obj->lock), TIMEOUT_BLOCK);
	status = msg_cancel(p_msg_obj->p_msg);

	if(status == ESUCCESS) {
		p_msg_obj->p_msg = NULL;
	}

	pm_rls_mutex(&(p_msg_obj->lock));

	if(status == ESUCCESS) {
		vfs_dec_obj_reference((pkobject_t)p_msg_obj);
	}

	pm_set_errno(status);
	return status;
}

k_status ssddt_read_msg(va_list p_args)
{
	//Agruments
	pmsg_obj_t p_msg_obj;
	pmsg_t buf;
	size_t size;

	//Variables

	//Get args
	p_msg_obj = va_arg(p_args, void*);
	buf = va_arg(p_args, pmsg_t);
	size = va_arg(p_args, size_t);

	//Check arguments
	if(!mm_virt_test(p_msg_obj, sizeof(msg_obj_t), PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return;
	}

	if(p_msg_obj->obj.name != msg_obj_name) {
		pm_set_errno(EINVAL);
		return;
	}

	if(p_msg_obj->p_msg == NULL) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	rtl_memcpy(buf, p_msg_obj->p_msg,
	           (size > p_msg_obj->p_msg.size ? p_msg_obj->p_msg.size : size));

	if(size > p_msg_obj->p_msg.size) {
		pm_set_errno(ESUCCESS);
		return ESUCCESS;

	} else {
		pm_set_errno(EOVERFLOW);
		return EOVERFLOW;
	}
}

void msg_obj_destroyer(pkobject_t p_obj)
{
	pmsg_obj_t p_msgobj = (pmsg_obj_t)p_obj;

	if(p_msgobj->p_msg != NULL) {
		msg_cancel(p_msgobj->p_msg);
	}

	mm_hp_free(p_obj, NULL);

	return;
}
