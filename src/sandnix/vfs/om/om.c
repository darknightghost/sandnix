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
#include "devfs/devfs.h"

static	array_list_t	drivers_list;
static	mutex_t			drivers_list_lock;
static	array_list_t	devices_list;
static	hash_table_t	dev_mj_index;
static	mutex_t			devices_list_lock;
static	hash_table_t	dev_filename_index;
static	mutex_t			dev_filename_index_lock;

u32		devfs_driver;

static	u32				dev_mj_hash(char* name);
static	bool			dev_mj_name_cmp(char* p_name1,
                                        char* p_name2);
static	void			driver_destroyer(pdriver_obj_t p_obj);
static	void			device_destroyer(pdevice_obj_t p_dev);

void om_init()
{
	dbg_print("Initializing Object Manager...\n");

	rtl_array_list_init(&drivers_list, DEV_MJ_NUM_MAX, NULL);
	rtl_array_list_init(&devices_list, DEV_MJ_NUM_MAX, NULL);
	rtl_hash_table_init(&dev_mj_index,
	                    0,
	                    0x0000ffff,
	                    (hash_func_t)dev_mj_hash,
	                    (compare_func_t)dev_mj_name_cmp,
	                    NULL);
	rtl_hash_table_init(&dev_filename_index,
	                    0,
	                    0x0000ffff,
	                    (hash_func_t)dev_mj_hash,
	                    (compare_func_t)dev_mj_name_cmp,
	                    NULL);

	pm_init_mutex(&drivers_list_lock);
	pm_init_mutex(&devices_list_lock);
	pm_init_mutex(&dev_filename_index_lock);

	vfs_get_dev_major_by_name("bus", DEV_TYPE_CHAR);
	vfs_get_dev_major_by_name("bus_dev_info", DEV_TYPE_CHAR);
	vfs_get_dev_major_by_name("dma", DEV_TYPE_CHAR);
	vfs_get_dev_major_by_name("memory", DEV_TYPE_CHAR);
	vfs_get_dev_major_by_name("ramdisk", DEV_TYPE_BLOCK);
	vfs_get_dev_major_by_name("tty", DEV_TYPE_CHAR);
	vfs_get_dev_major_by_name("floppy", DEV_TYPE_BLOCK);
	vfs_get_dev_major_by_name("ata", DEV_TYPE_BLOCK);
	vfs_get_dev_major_by_name("sata", DEV_TYPE_BLOCK);
	vfs_get_dev_major_by_name("loop", DEV_TYPE_BLOCK);
	vfs_get_dev_major_by_name("console", DEV_TYPE_CHAR);
	vfs_get_dev_major_by_name("partition", DEV_TYPE_BLOCK);
	vfs_get_dev_major_by_name("filesystem", DEV_TYPE_CHAR);
	vfs_get_dev_major_by_name("volume", DEV_TYPE_CHAR);

	devfs_init();

	return;
}

//Objects
void vfs_initialize_object(pkobject_t p_object)
{
	p_object->ref_count = 1;
	p_object->name = NULL;
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

	if(len > NAME_MAX + 1) {
		pm_set_errno(ENAMETOOLONG);
		return NULL;
	}

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
	ret->obj.class_num = OBJ_MJ_DRIVER;

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

	ret->destroy_flag = false;

	pm_set_errno(ESUCCESS);
	return ret;
}

u32 vfs_reg_driver(pdriver_obj_t p_driver)
{
	u32 new_id;

	if(has_drv_object()) {
		pm_set_errno(EEXIST);
		return 0;
	}

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

	p_driver->driver_id = new_id;

	pm_rls_mutex(&drivers_list_lock);
	set_drv_obj(new_id);
	return new_id;
}

k_status vfs_send_drv_message(u32 src_driver,
                              u32 dest_driver,
                              pmsg_t p_msg,
                              u32* p_result,
                              k_status* p_complete_result)
{
	pdriver_obj_t p_dest_drv, p_src_drv;

	p_src_drv = get_driver(src_driver);

	if(!OPERATE_SUCCESS) {
		return pm_get_errno();
	}

	p_dest_drv = get_driver(dest_driver);

	if(!OPERATE_SUCCESS) {
		return pm_get_errno();
	}

	p_msg->file_id = INVALID_FILEID;
	p_msg->src_thread = pm_get_crrnt_thrd_id();
	p_msg->result_queue = p_src_drv->msg_queue;

	return msg_send(p_msg, p_dest_drv->msg_queue, p_result, p_complete_result);
}

k_status vfs_recv_drv_message(u32 drv_num, pmsg_t* p_p_msg, bool if_block)
{
	pdriver_obj_t p_drv;

	p_drv = get_driver(drv_num);

	if(!OPERATE_SUCCESS) {
		return pm_get_errno();
	}

	return msg_recv(p_p_msg, p_drv->msg_queue, if_block);
}

//Device objects
pdevice_obj_t vfs_create_dev_object(char* dev_name)
{
	pdevice_obj_t ret;
	size_t len;

	len = rtl_strlen(dev_name) + 1;

	if(len > NAME_MAX + 1) {
		pm_set_errno(ENAMETOOLONG);
		return NULL;
	}

	ret = mm_hp_alloc(sizeof(device_obj_t), NULL);

	if(ret == NULL) {
		pm_set_errno(EFAULT);
		return NULL;
	}

	//Kernel object
	vfs_initialize_object((pkobject_t)ret);
	ret->file_obj.obj.size = sizeof(device_obj_t);

	ret->file_obj.obj.name = mm_hp_alloc(len, NULL);

	if(ret->file_obj.obj.name == NULL) {
		mm_hp_free(ret, NULL);
		pm_set_errno(EFAULT);
		return NULL;
	}

	rtl_strcpy_s(ret->file_obj.obj.name, len, dev_name);
	ret->file_obj.obj.class_num = OBJ_MJ_FILE | OBJ_MN_DEVICE;
	ret->file_obj.obj.destroy_callback = (obj_destroyer)device_destroyer;

	//File object
	ret->child_list = NULL;
	ret->file_obj.refered_proc_list = NULL;
	ret->has_parent = false;
	pm_init_mutex(&(ret->child_list_lock));
	pm_init_mutex(&(ret->file_obj.refered_proc_list_lock));

	ret->p_additional = NULL;
	ret->additional_destroyer = NULL;

	return ret;
}

u32 vfs_add_device(pdevice_obj_t p_device, u32 driver)
{
	u32 new_dev_num;
	pdriver_obj_t p_drv_obj;
	pdev_mj_info_t p_mj_info;
	u32	minor_num;
	pdevice_obj_t p_parent_dev;
	plist_node_t p_lst_node;

	//Get the driver
	p_drv_obj = get_driver(driver);

	if(p_drv_obj == NULL) {
		pm_set_errno(EINVAL);
		return 0;
	}

	pm_acqr_mutex(&devices_list_lock, TIMEOUT_BLOCK);

	//Get major number exists
	p_mj_info = rtl_array_list_get(&devices_list,
	                               DEV_NUM_MJ(p_device->device_number));

	if(p_mj_info == NULL) {
		vfs_dec_obj_reference((pkobject_t)p_drv_obj);
		pm_rls_mutex(&devices_list_lock);
		pm_set_errno(EINVAL);
		return 0;
	}

	pm_rls_mutex(&devices_list_lock);

	pm_acqr_mutex(&(p_mj_info->lock), TIMEOUT_BLOCK);

	//Add device
	minor_num = rtl_array_list_get_free_index(&(p_mj_info->devices));

	if(!OPERATE_SUCCESS) {
		vfs_dec_obj_reference((pkobject_t)p_drv_obj);
		pm_rls_mutex(&(p_mj_info->lock));
		pm_set_errno(ENOMEM);
		return 0;
	}

	new_dev_num = MK_DEV(DEV_NUM_MJ(p_device->device_number), minor_num);
	p_device->device_number = new_dev_num;


	rtl_array_list_set(&(p_mj_info->devices), minor_num, p_device, NULL);

	if(!OPERATE_SUCCESS) {
		vfs_dec_obj_reference((pkobject_t)p_drv_obj);
		pm_rls_mutex(&(p_mj_info->lock));
		pm_set_errno(ENOMEM);
		return 0;
	}

	(p_mj_info->devices_count)++;
	pm_rls_mutex(&(p_mj_info->lock));

	//Add to driver
	pm_acqr_mutex(&(p_drv_obj->file_list_lock), TIMEOUT_BLOCK);

	p_lst_node = rtl_list_insert_after(&(p_drv_obj->file_list),
	                                   NULL,
	                                   p_device,
	                                   NULL);

	if(p_lst_node == NULL) {
		pm_rls_mutex(&(p_drv_obj->file_list_lock));

		pm_acqr_mutex(&(p_mj_info->lock), TIMEOUT_BLOCK);
		rtl_array_list_release(&(p_mj_info->devices), minor_num, NULL);
		(p_mj_info->devices_count)--;
		pm_rls_mutex(&(p_mj_info->lock));

		vfs_dec_obj_reference((pkobject_t)p_drv_obj);
		pm_set_errno(EFAULT);
		return 0;
	}

	pm_rls_mutex(&(p_drv_obj->file_list_lock));

	if(p_device->has_parent) {
		//Get parent device
		p_parent_dev = get_dev(p_device->parent_dev);

		if(p_parent_dev == NULL) {
			pm_acqr_mutex(&(p_drv_obj->file_list_lock), TIMEOUT_BLOCK);
			rtl_list_remove(&(p_drv_obj->file_list), p_lst_node, NULL);
			pm_rls_mutex(&(p_drv_obj->file_list_lock));

			pm_acqr_mutex(&(p_mj_info->lock), TIMEOUT_BLOCK);
			rtl_array_list_release(&(p_mj_info->devices), minor_num, NULL);
			(p_mj_info->devices_count)--;
			pm_rls_mutex(&(p_mj_info->lock));

			vfs_dec_obj_reference((pkobject_t)p_drv_obj);
			pm_set_errno(EFAULT);
			return 0;
		}

		//Add to parent
		pm_acqr_mutex(&(p_parent_dev->child_list_lock), TIMEOUT_BLOCK);

		if(rtl_list_insert_after(&(p_parent_dev->child_list),
		                         NULL,
		                         p_device,
		                         NULL) == NULL) {
			pm_rls_mutex(&(p_parent_dev->child_list_lock));

			pm_acqr_mutex(&(p_drv_obj->file_list_lock), TIMEOUT_BLOCK);
			rtl_list_remove(&(p_drv_obj->file_list), p_lst_node, NULL);
			pm_rls_mutex(&(p_drv_obj->file_list_lock));

			pm_acqr_mutex(&(p_mj_info->lock), TIMEOUT_BLOCK);
			rtl_array_list_release(&(p_mj_info->devices), minor_num, NULL);
			(p_mj_info->devices_count)--;
			pm_rls_mutex(&(p_mj_info->lock));

			vfs_dec_obj_reference((pkobject_t)p_drv_obj);
			pm_set_errno(EFAULT);
			return 0;
		}

		pm_rls_mutex(&(p_parent_dev->child_list_lock));
	}

	add_file_obj((pfile_obj_t)p_device);

	return new_dev_num;
}

void vfs_remove_device(u32 device)
{
	pdevice_obj_t p_dev, p_parent_dev;
	plist_node_t p_node;

	p_dev = get_dev(device);

	if(!OPERATE_SUCCESS) {
		return;
	}

	if(p_dev->has_parent) {
		//Parent
		p_parent_dev = get_dev(p_dev->parent_dev);

		if(!OPERATE_SUCCESS) {
			return;
		}

		pm_acqr_mutex(&(p_parent_dev->child_list_lock), TIMEOUT_BLOCK);
		p_node = p_parent_dev->child_list;

		if(p_node != NULL) {
			do {
				if(p_node->p_item == p_dev) {
					rtl_list_remove(&(p_parent_dev->child_list),
					                p_node,
					                NULL);
					break;
				}
			} while(p_node != p_parent_dev->child_list);
		}

		pm_rls_mutex(&(p_parent_dev->child_list_lock));
	}

	//Childs
	for(p_node = p_dev->child_list;
	    p_node != NULL;
	    p_node = p_dev->child_list) {
		vfs_remove_device(((pdevice_obj_t)(p_node->p_item))->device_number);
	}

	remove_file_obj((pfile_obj_t)p_dev);

	if(p_dev->file_obj.obj.name != NULL) {
		pm_acqr_mutex(&dev_filename_index_lock, TIMEOUT_BLOCK);
		rtl_hash_table_remove(&dev_filename_index,
		                      p_dev->file_obj.obj.name,
		                      NULL);
		pm_rls_mutex(&dev_filename_index_lock);
	}

	vfs_dec_obj_reference((pkobject_t)p_dev);
	return;
}

k_status vfs_set_dev_filename(u32 device, char* name)
{
	size_t len;
	pdevice_obj_t p_dev;

	len = rtl_strlen(name) + 1;
	p_dev = get_dev(device);

	if(len > NAME_MAX + 1) {
		pm_set_errno(ENAMETOOLONG);
		return ENAMETOOLONG;
	}

	if(p_dev == NULL) {
		return pm_get_errno();
	}

	//Free old filename
	if(p_dev->file_obj.obj.name != NULL) {

		pm_acqr_mutex(&dev_filename_index_lock, TIMEOUT_BLOCK);
		rtl_hash_table_remove(&dev_filename_index,
		                      p_dev->file_obj.obj.name,
		                      NULL);
		pm_rls_mutex(&dev_filename_index_lock);

		mm_hp_free(p_dev->file_obj.obj.name, NULL);
	}

	//Allocate memory for new name
	p_dev->file_obj.obj.name = mm_hp_alloc(len,
	                                       NULL);

	if(p_dev->file_obj.obj.name == NULL) {
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	rtl_strcpy_s(p_dev->file_obj.obj.name, len , name);

	pm_acqr_mutex(&dev_filename_index_lock, TIMEOUT_BLOCK);
	rtl_hash_table_set(&dev_filename_index,
	                   p_dev->file_obj.obj.name,
	                   p_dev,
	                   NULL);
	pm_rls_mutex(&dev_filename_index_lock);

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

k_status vfs_send_dev_message(u32 src_driver,
                              u32 dest_dev,
                              pmsg_t p_msg,
                              u32* p_result,
                              k_status* p_complete_result)
{
	pdriver_obj_t p_src_drv;
	pdevice_obj_t p_dest_dev;

	p_src_drv = get_driver(src_driver);

	if(!OPERATE_SUCCESS) {
		return pm_get_errno();
	}

	p_dest_dev = get_dev(dest_dev);

	if(!OPERATE_SUCCESS) {
		return pm_get_errno();
	}

	p_msg->file_id = p_dest_dev->file_obj.file_id;
	p_msg->src_thread = pm_get_crrnt_thrd_id();
	p_msg->result_queue = p_src_drv->msg_queue;

	return msg_send(p_msg,
	                p_dest_dev->file_obj.p_driver->msg_queue,
	                p_result,
	                p_complete_result);
}

u32 vfs_get_dev_major_by_name(char* major_name, u32 type)
{
	pdev_mj_info_t p_info;
	u32 major;
	size_t len;

	len = rtl_strlen(major_name) + 1;

	if(len > NAME_MAX + 1) {
		pm_set_errno(ENAMETOOLONG);
		return ENAMETOOLONG;
	}

	pm_acqr_mutex(&devices_list_lock, TIMEOUT_BLOCK);
	p_info = rtl_hash_table_get(&dev_mj_index, major_name);

	if(p_info == NULL) {
		//If the major number not exists
		//Allocate new dev_mj_info_t
		p_info = mm_hp_alloc(sizeof(dev_mj_info_t), NULL);

		if(p_info == NULL) {
			pm_rls_mutex(&devices_list_lock);
			pm_set_errno(EFAULT);
			return 0;
		}

		//Object
		vfs_initialize_object((pkobject_t)(&(p_info->file_obj.obj)));
		p_info->file_obj.p_driver = get_driver(devfs_driver);
		p_info->file_obj.refered_proc_list = NULL;
		p_info->file_obj.obj.class_num = OBJ_MJ_FILE | OBJ_MN_NORMAL;
		pm_init_mutex(&(p_info->file_obj.refered_proc_list_lock));

		p_info->file_obj.obj.name = mm_hp_alloc(len, NULL);

		if(p_info->file_obj.obj.name == NULL) {
			pm_rls_mutex(&devices_list_lock);
			mm_hp_free(p_info, NULL);
			pm_set_errno(EFAULT);
			return 0;
		}


		rtl_strcpy_s(p_info->file_obj.obj.name, len, major_name);

		rtl_array_list_init((&p_info->devices), DEV_MN_NUM_MAX, NULL);

		if(!OPERATE_SUCCESS) {
			pm_rls_mutex(&devices_list_lock);
			mm_hp_free(p_info->file_obj.obj.name, NULL);
			mm_hp_free(p_info, NULL);
			pm_set_errno(EFAULT);
			return 0;
		}

		pm_init_mutex(&p_info->lock);

		//Get new major
		major = rtl_array_list_get_free_index(&devices_list);

		p_info->mj_num = major;
		p_info->device_type = type;
		p_info->devices_count = 0;

		//Add to devices_list
		rtl_array_list_set(&devices_list, major, p_info, NULL);

		if(!OPERATE_SUCCESS) {
			pm_rls_mutex(&devices_list_lock);
			rtl_array_list_destroy(&(p_info->devices), NULL, NULL, NULL);
			mm_hp_free(p_info->file_obj.obj.name, NULL);
			mm_hp_free(p_info, NULL);
			pm_set_errno(EFAULT);
			return 0;
		}

		//Add to hash table
		rtl_hash_table_set(&dev_mj_index,
		                   p_info->file_obj.obj.name,
		                   p_info,
		                   NULL);

		if(!OPERATE_SUCCESS) {
			rtl_array_list_release(&devices_list, major, NULL);
			pm_rls_mutex(&devices_list_lock);
			rtl_array_list_destroy(&(p_info->devices), NULL, NULL, NULL);
			mm_hp_free(p_info->file_obj.obj.name, NULL);
			mm_hp_free(p_info, NULL);
			pm_set_errno(EFAULT);
			return 0;
		}

		add_file_obj((pfile_obj_t)p_info);
	}

	pm_rls_mutex(&devices_list_lock);

	pm_set_errno(ESUCCESS);
	return major;
}

k_status vfs_msg_forward(pmsg_t p_msg, u32 dev_num)
{
	pdevice_obj_t p_dev_obj;

	p_dev_obj = (pdevice_obj_t)get_dev(dev_num);

	if(!OPERATE_SUCCESS) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	return msg_forward(p_msg, p_dev_obj->file_obj.p_driver->msg_queue);
}

void vfs_sync(u32 dev_num)
{
	pmsg_t p_msg;
	k_status status;
	k_status complete_status;
	u32 result;

	status = msg_create(&p_msg, sizeof(msg_t));

	if(status != ESUCCESS) {
		pm_set_errno(status);
		return;
	}

	p_msg->flags.flags = 0;
	p_msg->message = MSG_SYNC;

	status = vfs_send_dev_message(
	             kernel_drv_num,
	             dev_num,
	             p_msg,
	             &result,
	             &complete_status);

	if(status != ESUCCESS) {
		pm_set_errno(status);
		return;
	}

	if(result != MSTATUS_COMPLETE) {
		pm_set_errno(ENODEV);
		return;
	}

	pm_set_errno(complete_status);
	return;
}

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

bool dev_mj_name_cmp(char* p_name1, char* p_name2)
{
	if(rtl_strcmp(p_name1, p_name2) == 0) {
		return true;
	}

	return false;
}

void driver_destroyer(pdriver_obj_t p_obj)
{
	plist_node_t p_node;

	for(p_node = p_obj->file_list;
	    p_node != NULL;
	    p_node = p_obj->file_list) {
		vfs_remove_device(((pdevice_obj_t)(p_node->p_item))->device_number);
	}

	pm_acqr_mutex(&drivers_list_lock, TIMEOUT_BLOCK);

	rtl_array_list_release(&drivers_list, p_obj->driver_id, NULL);

	pm_rls_mutex(&drivers_list_lock);
	msg_queue_destroy(p_obj->msg_queue);
	mm_hp_free(p_obj, NULL);

	return;
}

void device_destroyer(pdevice_obj_t p_dev)
{
	pdev_mj_info_t p_info;
	pdriver_obj_t p_drv;
	plist_node_t p_node;

	//Send message
	if(!p_dev->file_obj.p_driver->destroy_flag) {
		send_file_obj_destroy_msg((pfile_obj_t)p_dev);
	}

	if(p_dev->additional_destroyer != NULL) {
		p_dev->additional_destroyer((pkobject_t)p_dev);
	}

	//Remove devices
	//Major
	pm_acqr_mutex(&devices_list_lock, TIMEOUT_BLOCK);
	p_info = rtl_array_list_get(&devices_list, DEV_NUM_MJ(p_dev->device_number));

	if(!OPERATE_SUCCESS) {
		pm_rls_mutex(&devices_list_lock);
		mm_hp_free(p_dev, NULL);
		return;
	}

	pm_rls_mutex(&devices_list_lock);

	//Minor
	pm_acqr_mutex(&(p_info->lock), TIMEOUT_BLOCK);

	rtl_array_list_release(&(p_info->devices),
	                       DEV_NUM_MN(p_dev->device_number),
	                       NULL);
	(p_info->devices_count)--;

	//Remove from driver
	p_drv = p_dev->file_obj.p_driver;
	pm_acqr_mutex(&(p_drv->file_list_lock), TIMEOUT_BLOCK);
	p_node = p_drv->file_list;

	if(p_node != NULL) {
		do {
			if(p_node->p_item == p_dev) {
				rtl_list_remove(&(p_drv->file_list),
				                p_node,
				                NULL);
				break;
			}
		} while(p_node != p_drv->file_list);
	}

	pm_rls_mutex(&(p_drv->file_list_lock));

	if(p_dev->file_obj.obj.name != NULL) {
		mm_hp_free(p_dev->file_obj.obj.name, NULL);
	}

	mm_hp_free(p_dev, NULL);

	pm_rls_mutex(&(p_info->lock));

	return;
}

pdevice_obj_t get_dev(u32 dev_num)
{
	pdev_mj_info_t p_info;
	pdevice_obj_t p_dev;

	//Major
	pm_acqr_mutex(&devices_list_lock, TIMEOUT_BLOCK);
	p_info = rtl_array_list_get(&devices_list, DEV_NUM_MJ(dev_num));

	if(!OPERATE_SUCCESS) {
		pm_rls_mutex(&devices_list_lock);
		return NULL;
	}

	pm_rls_mutex(&devices_list_lock);

	//Minor
	pm_acqr_mutex(&(p_info->lock), TIMEOUT_BLOCK);

	p_dev = rtl_array_list_get(&(p_info->devices), TIMEOUT_BLOCK);

	if(!OPERATE_SUCCESS) {
		pm_rls_mutex(&(p_info->lock));
		return NULL;
	}

	pm_rls_mutex(&(p_info->lock));

	pm_set_errno(ESUCCESS);
	return p_dev;
}

pdriver_obj_t get_driver(u32 driver_id)
{
	pdriver_obj_t p_drv_obj;

	pm_acqr_mutex(&drivers_list_lock, TIMEOUT_BLOCK);
	p_drv_obj = rtl_array_list_get(&drivers_list, driver_id);

	if(p_drv_obj == NULL) {
		pm_rls_mutex(&drivers_list_lock);
		pm_set_errno(EINVAL);
		return 0;
	}

	pm_rls_mutex(&drivers_list_lock);

	return p_drv_obj;
}

pdevice_obj_t get_dev_by_name(char* name)
{
	pdevice_obj_t p_dev;

	pm_acqr_mutex(&dev_filename_index_lock, TIMEOUT_BLOCK);
	p_dev = rtl_hash_table_get(&dev_filename_index, name);
	pm_rls_mutex(&dev_filename_index_lock);

	return p_dev;
}

pdev_mj_info_t get_mj_by_name(char* name)
{
	pdev_mj_info_t p_info;

	pm_acqr_mutex(&devices_list_lock, TIMEOUT_BLOCK);
	p_info = rtl_hash_table_get(&dev_mj_index, name);
	pm_rls_mutex(&devices_list_lock);

	return p_info;
}

size_t get_devfs_root(pdirent_t buf, size_t offset, size_t count)
{
	size_t read_len;
	u32 mj_num;
	u32 mn_num;
	pdirent_t p_data;
	pdev_mj_info_t p_info;
	pdevice_obj_t p_dev;

	read_len = 0;
	p_data = buf;

	pm_acqr_mutex(&devices_list_lock, TIMEOUT_BLOCK);

	for(mj_num = rtl_array_list_get_next_index(&devices_list, 0);
	    OPERATE_SUCCESS;
	    mj_num = rtl_array_list_get_next_index(&devices_list, mj_num + 1)) {

		p_info = rtl_array_list_get(&devices_list, mj_num);

		if(read_len + sizeof(dirent_t) +
		   rtl_strlen(p_info->file_obj.obj.name) > count) {
			break;
		}

		if(offset == 0) {
			//Directory
			p_data->d_ino = 0 - mj_num;
			rtl_strcpy_s(&(p_data->d_name), NAME_MAX, p_info->file_obj.obj.name);
			p_data->d_off = read_len;
			p_data->d_reclen = rtl_strlen(&(p_data->d_name)) + 1;
			read_len += sizeof(dirent_t) + p_data->d_reclen - 1;
			p_data = (pdirent_t)((u8*)p_data + sizeof(dirent_t) + p_data->d_reclen - 1);

		} else {
			offset -= sizeof(dirent_t) +
			          rtl_strlen(p_info->file_obj.obj.name);
		}

		//Named devices
		pm_acqr_mutex(&(p_info->lock), TIMEOUT_BLOCK);

		for(mn_num = rtl_array_list_get_next_index(&(p_info->devices), 0);
		    OPERATE_SUCCESS;
		    mn_num = rtl_array_list_get_next_index(&(p_info->devices), mn_num + 1)) {

			p_dev = rtl_array_list_get(&(p_info->devices), mn_num);

			if(read_len + sizeof(dirent_t) +
			   rtl_strlen(p_dev->file_obj.obj.name) > count) {
				break;
			}

			if(offset > 0) {
				offset -= sizeof(dirent_t) +
				          rtl_strlen(p_dev->file_obj.obj.name);

			} else {
				if(p_dev->file_obj.obj.name != NULL) {
					p_data->d_ino = p_dev->device_number;
					p_data->d_off = read_len;
					rtl_strcpy_s(&(p_data->d_name), NAME_MAX, p_dev->file_obj.obj.name);
					p_data->d_reclen = rtl_strlen(&(p_data->d_name)) + 1;
					read_len += sizeof(dirent_t) + p_data->d_reclen - 1;
					p_data = (pdirent_t)((u8*)p_data + sizeof(dirent_t) + p_data->d_reclen - 1);
				}
			}
		}

		pm_rls_mutex(&(p_info->lock));
	}

	pm_rls_mutex(&devices_list_lock);

	pm_set_errno(ESUCCESS);
	return read_len;
}

size_t get_devfs_dir(pdev_mj_info_t p_dir, pdirent_t buf,
                     size_t offset, size_t count)
{
	pdevice_obj_t p_dev;
	u32 index;
	pdirent_t p_info;
	size_t read_len;
	char *p;
	char name_buf[10];

	pm_acqr_mutex(&(p_dir->lock), TIMEOUT_BLOCK);
	read_len = 0;
	p_info = buf;

	for(index = rtl_array_list_get_next_index(&(p_dir->devices), 0);
	    OPERATE_SUCCESS;
	    index = rtl_array_list_get_next_index(&(p_dir->devices), index + 1)) {

		p_dev = rtl_array_list_get(&(p_dir->devices), index);

		//Filename
		rtl_itoa(name_buf, DEV_NUM_MJ(p_dev->device_number));
		p = name_buf + rtl_strlen(name_buf);
		*p = ':';
		p++;
		rtl_itoa(p, DEV_NUM_MN(p_dev->device_number));

		//Jump read inodes
		if(offset > 0) {
			offset -= sizeof(dirent_t) + rtl_strlen(name_buf);
			continue;
		}

		if(read_len + sizeof(dirent_t) + rtl_strlen(name_buf) >= count) {
			break;
		}

		//Inode
		p_info->d_ino = p_dev->device_number;
		p_info->d_off = read_len;
		rtl_strcpy_s(&(p_info->d_name), NAME_MAX, name_buf);
		p_info->d_reclen = rtl_strlen(&(p_info->d_name)) + 1;

		read_len += sizeof(dirent_t) + p_info->d_reclen - 1;
		p_info++;
	}

	pm_rls_mutex(&(p_dir->lock));
	return read_len;
}
