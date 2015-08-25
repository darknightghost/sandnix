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

#include "../vfs.h"
#include "../../rtl/rtl.h"
#include "../../debug/debug.h"
#include "../../exceptions/exceptions.h"
#include "../../pm/pm.h"

static	array_list_t	drivers_list;
static	mutex_t			drivers_list_lock;
static	array_list_t	devices_list;
static	hash_table_t	dev_mj_index;
static	mutex_t			devices_list_lock;

static	u32			dev_mj_hash(char* name);
static	bool		dev_mj_name_cmp(pdev_mj_info_t p_dev1, pdev_mj_info_t p_dev2);
static	void		driver_destroyer(pdriver_obj_t p_obj);
static	void		device_destroyer(pdevice_obj_t p_dev);

void om_init()
{
	dbg_print("\nInitializing Object Manager...\n");

	rtl_array_list_init(&drivers_list, DEV_MJ_NUM_MAX, NULL);
	rtl_array_list_init(&devices_list, DEV_MJ_NUM_MAX, NULL);
	rtl_hash_table_init(&dev_mj_index,
	                    0,
	                    0x0000FFFF,
	                    (hash_func_t)dev_mj_hash,
	                    (compare_func_t)dev_mj_name_cmp,
	                    NULL);
	pm_init_mutex(&drivers_list_lock);
	pm_init_mutex(&devices_list_lock);

	return;
}

//Objects
void vfs_initialize_object(pkobject_t p_object)
{
	p_object->ref_count = 1;
	pm_init_mutex(&(p_object->ref_count_lock));
	return;
}

void vfs_inc_obj_reference(pkobject_t p_object)
{
	pm_acqr_mutex(&(p_object->ref_count_lock), TIMEOUT_BLOCK);
	(p_object->ref_count)++;
	pm_rls_mutex(&(p_object->ref_count_lock));
	pm_set_errno(ESUCCESS);
	return;
}

void vfs_dec_obj_reference(pkobject_t p_object)
{
	pm_acqr_mutex(&(p_object->ref_count_lock), TIMEOUT_BLOCK);
	(p_object->ref_count)--;

	if(p_object->ref_count == 0) {
		//Destroy the object
		p_object->destroy_callback(p_object);
	}

	pm_rls_mutex(&(p_object->ref_count_lock));
	pm_set_errno(ESUCCESS);
	return;
}

//Driver Objects
pdriver_obj_t vfs_create_drv_object(char* drv_name)
{
	pdriver_obj_t ret;
	size_t len;

	len = rtl_strlen(drv_name) + 1;

	//Allocate memory
	ret = mm_hp_alloc(sizeof(driver_obj_t), NULL);

	if(ret == NULL) {
		pm_set_errno(EFAULT);
		return NULL;
	}

	vfs_initialize_object((pkobject_t)ret);

	//Kernel object
	ret->obj.name = mm_hp_alloc(len, NULL);

	if(ret->obj.name == NULL) {
		mm_hp_free(ret, NULL);
		pm_set_errno(EFAULT);
		return NULL;
	}

	rtl_strcpy_s(ret->obj.name, len, drv_name);

	ret->obj.size = sizeof(driver_obj_t);
	ret->obj.destroy_callback = (obj_destroyer)driver_destroyer;
	ret->obj.class = OBJ_MJ_DRIVER;

	//Driver object
	ret->file_list = NULL;
	pm_init_mutex(&(ret->file_list_lock));
	ret->msg_queue = msg_queue_create();

	if(!OPERATE_SUCCESS) {
		mm_hp_free(ret->obj.name, NULL);
		mm_hp_free(ret, NULL);
		pm_set_errno(EFAULT);
		return NULL;
	}

	pm_set_errno(ESUCCESS);
	return ret;
}

u32 vfs_reg_driver(pdriver_obj_t p_driver)
{
	u32 new_id;
	pm_acqr_mutex(&drivers_list_lock, TIMEOUT_BLOCK);

	//Get id
	new_id = rtl_array_list_get_free_index(&drivers_list);

	if(!OPERATE_SUCCESS) {
		pm_rls_mutex(&drivers_list_lock);
		return 0;
	}

	//Set Value
	if(rtl_array_list_set(&drivers_list,
	                      new_id,
	                      p_driver,
	                      NULL) != ESUCCESS) {
		pm_rls_mutex(&drivers_list_lock);
		return 0;
	}

	pm_rls_mutex(&drivers_list_lock);
	return new_id;
}

k_status		vfs_send_drv_message(u32 dest_driver,
                                     pmsg_t p_msg);
k_status		vfs_recv_drv_message(pmsg_t buf);

//Device objects
pdevice_obj_t	vfs_create_dev_object(char* dev_name)
{
	pdevice_obj_t ret;
	size_t len;

	ret = mm_hp_alloc(sizeof(device_obj_t), NULL);

	if(ret == NULL) {
		pm_set_errno(EFAULT);
		return NULL;
	}

	//Kernel object
	vfs_initialize_object((pkobject_t)ret);
	ret->file_obj.obj.size = sizeof(device_obj_t);

	len = rtl_strlen(dev_name);
	ret->file_obj.obj.name = mm_hp_alloc(len, NULL);

	if(ret->file_obj.obj.name == NULL) {
		mm_hp_free(ret, NULL);
		pm_set_errno(EFAULT);
		return NULL;
	}

	rtl_strcpy_s(ret->file_obj.obj.name, len, dev_name);
	ret->file_obj.obj.class = OBJ_MJ_FILE | OBJ_MN_DEVICE;
	ret->file_obj.obj.destroy_callback = (obj_destroyer)device_destroyer;

	//File object
	ret->file_obj.child_list = NULL;
	ret->file_obj.refered_proc_list = NULL;
	pm_init_mutex(&(ret->file_obj.child_list_lock));
	pm_init_mutex(&(ret->file_obj.refered_proc_list_lock));

	return ret;
}

u32				vfs_add_device(pdevice_obj_t p_device, u32 driver);
void			vfs_remove_device(u32 device);

k_status		vfs_send_dev_message(u32 dest_dev,
                                     pmsg_t p_msg);
u32				vfs_get_dev_major_by_name(char* major_name);

u32 dev_mj_hash(char* name)
{
	u32 hash = 0;
	u32 i;
	u32 len;

	len = rtl_strlen(name);

	for(i = 0; i < len; i++) {
		hash = 33 * hash + *(name + i);
	}

	return hash % 0x00010000;
}

bool dev_mj_name_cmp(pdev_mj_info_t p_dev1, pdev_mj_info_t p_dev2)
{
	if(rtl_strcmp(p_dev1->name, p_dev2->name) == 0) {
		return true;
	}

	return false;
}

void driver_destroyer(pdriver_obj_t p_obj)
{
	//TODO:
	UNREFERRED_PARAMETER(p_obj);
}

void device_destroyer(pdevice_obj_t p_dev)
{
	//TODO:
	UNREFERRED_PARAMETER(p_dev);
}
