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
#include "../../pm/pm.h"
#include "../../rtl/rtl.h"
#include "../../exceptions/exceptions.h"
#include "ramdisk/ramdisk.h"
#include "tarfs/tarfs.h"
#include "../../debug/debug.h"
#include "../../io/io.h"

static	array_list_t	file_desc_info_table;
static	mutex_t			file_desc_info_table_lock;
static	array_list_t	file_obj_table;
static	mutex_t			file_obj_table_lock;

static	mount_point_t	root_mount_point;
static	mutex_t			mount_point_lock;

static	pvfs_proc_info	get_proc_fs_info();
static	void			set_proc_fs_info(u32 process_id,
        pvfs_proc_info p_new_info);
static	void			ref_proc_destroy_callback(pfile_obj_ref_t p_item,
        pfile_obj_t p_file_object);
static	void			file_desc_destroy_callback(pfile_desc_t p_fd,
        void* p_null);
static	k_status		analyse_path(char* path, ppath_t ret);
static	pfile_desc_t	get_file_descriptor(u32 fd);
static	void			file_objecj_release_callback(pfile_obj_t p_fo);
static	k_status		get_cwd(char* buf, size_t size);

void fs_init()
{
	k_status status;
	pvfs_proc_info p_proc0_info, p_proc_fd_info;
	pdriver_obj_t p_drv;
	u32 process_id;

	dbg_print("Initializing filesystem...\n");

	//Initialize tables
	status = rtl_array_list_init(&file_desc_info_table,
	                             MAX_PROCESS_NUM,
	                             NULL);

	if(status != ESUCCESS) {
		excpt_panic(status, "Failed to initialize file_desc_info_table.");
	}

	status = rtl_array_list_init(&file_obj_table,
	                             MAX_FILEOBJ_NUM,
	                             NULL);

	if(status != ESUCCESS) {
		excpt_panic(status, "Failed to initialize file_obj_table.");
	}

	pm_init_mutex(&file_desc_info_table_lock);
	pm_init_mutex(&mount_point_lock);

	//Initialize file descriptor for process 0
	p_proc0_info = mm_hp_alloc(sizeof(vfs_proc_info), NULL);

	if(p_proc0_info == NULL) {
		excpt_panic(EFAULT,
		            "Failed to initialize file descriptor for process 0");
	}

	rtl_memset(p_proc0_info, 0, sizeof(vfs_proc_info));
	pm_init_mutex(&(p_proc0_info->lock));

	status = rtl_array_list_set(&file_desc_info_table, 0, p_proc0_info, NULL);
	
	status=rtl_array_list_init(&(p_proc0_info->file_descs),
                                    1024,
                                    NULL);
	if(status!=ESUCCESS){
		excpt_panic(status,
		            "Failed to initialize file descriptor for process 0");
	}
	
	status=rtl_array_list_init(&(p_proc0_info->ref_objs),
                                    1024,
                                    NULL);
	if(status!=ESUCCESS){
		excpt_panic(status,
		            "Failed to initialize file descriptor for process 0");
	}

	if(status != ESUCCESS) {
		excpt_panic(status, "Failed to initialize file_desc_info_table.");
	}
	
	//Create driver object of process 0.
	p_drv = vfs_create_drv_object("kernel");

	if(!OPERATE_SUCCESS) {
		excpt_panic(status, "Failed to create kernel driver object.");
	}

	p_drv->process_id = 0;

	kernel_drv_num = vfs_reg_driver(p_drv);

	if(!OPERATE_SUCCESS) {
		excpt_panic(status, "Failed to regist kernel driver object.");
	}

	p_proc0_info->driver_obj = p_drv->driver_id;

	//Initialize ramdisk and tarfs
	ramdisk_init();
	tarfs_init();

	//Mount ramdisk as root filesytem
	dbg_print("Mounting ramdisk...\n");
	root_mount_point.path.p_mount_point = NULL;
	root_mount_point.path.path = "";
	root_mount_point.uid = 0;
	root_mount_point.gid = 0;
	root_mount_point.mode = S_IRUSR | S_IRGRP | S_IROTH;
	root_mount_point.p_parent = NULL;

	//Set root dir and work dir of all process
	pm_acqr_mutex(&file_desc_info_table_lock, TIMEOUT_BLOCK);

	for(process_id = rtl_array_list_get_next_index(&file_desc_info_table, 0);
	    OPERATE_SUCCESS;
	    process_id = rtl_array_list_get_next_index(&file_desc_info_table,
	                 process_id + 1)) {
		p_proc_fd_info = rtl_array_list_get(&file_desc_info_table, process_id);
		p_proc_fd_info->root.p_mount_point = &root_mount_point;
		p_proc_fd_info->root.path = mm_hp_alloc(1, NULL);

		if(p_proc_fd_info->root.path == NULL) {
			excpt_panic(EFAULT, "Failed to set root directory.");
		}

		*(p_proc_fd_info->root.path) = '\0';

		p_proc_fd_info->pwd.p_mount_point = &root_mount_point;
		p_proc_fd_info->pwd.path = mm_hp_alloc(1, NULL);

		if(p_proc_fd_info->pwd.path == NULL) {
			excpt_panic(EFAULT, "Failed to set work directory.");
		}

		*(p_proc_fd_info->pwd.path) = '\0';

	}

	pm_rls_mutex(&file_desc_info_table_lock);


	return;
}

//Volumes
k_status vfs_mount(char* src, char* target,
                   char* fs_type, u32 flags,
                   char* args)
{
	path_t k_path;
	pmsg_t p_msg;
	k_status status, complete_result;
	ppmo_t p_pmo;
	pmsg_mount_info_t p_info;
	pmount_point_t p_mount_point, p_parent_point;
	u32 send_result;
	ppmo_t p_stat_buf;
	pfile_stat_t p_stat;
	u32 uid;
	u32 gid;
	pdevice_obj_t p_dev;

	//Only root can mount
	if(!pm_get_proc_euid(pm_get_crrnt_process(), &uid) || uid != 0) {
		pm_set_errno(EACCES);
		return EACCES;
	}

	//Get folder stats
	p_stat_buf = mm_pmo_create(sizeof(file_stat_t));

	if(p_stat_buf == NULL) {
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	p_stat = mm_pmo_map(NULL, p_stat_buf, false);

	if(p_stat == NULL) {
		mm_pmo_free(p_stat_buf);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	status = vfs_stat(target, p_stat_buf);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_stat, p_stat_buf);
		mm_pmo_free(p_stat_buf);
		pm_set_errno(status);
		return status;
	}

	gid = p_stat->gid;
	uid = p_stat->uid;

	mm_pmo_unmap(p_stat, p_stat_buf);
	mm_pmo_free(p_stat_buf);

	//Get filesystem device
	p_dev = get_dev_by_name(fs_type);

	if(p_dev == NULL) {
		pm_set_errno(ENODEV);
		return ENODEV;
	}

	if(DEV_NUM_MJ(p_dev->device_number)
	   != vfs_get_dev_major_by_name("filesystem", DEV_TYPE_CHAR)) {
		pm_set_errno(ENODEV);
		return ENODEV;
	}

	//Analyse target path
	pm_acqr_mutex(&mount_point_lock, TIMEOUT_BLOCK);
	status = analyse_path(target, &k_path);

	if(status != ESUCCESS) {
		pm_rls_mutex(&mount_point_lock);
		return status;
	}

	//Create buffer
	p_pmo = mm_pmo_create(sizeof(msg_mount_info_t) + 2
	                      + rtl_strlen(src) + rtl_strlen(src));

	if(p_pmo == NULL) {
		pm_rls_mutex(&mount_point_lock);
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	p_info = mm_pmo_map(NULL, p_pmo, false);

	if(p_info == NULL) {
		pm_rls_mutex(&mount_point_lock);
		mm_pmo_free(p_pmo);
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	p_mount_point = mm_hp_alloc(sizeof(mount_point_t), NULL);

	if(p_mount_point == NULL) {
		pm_rls_mutex(&mount_point_lock);
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	//Create message
	status = msg_create(&p_msg,
	                    sizeof(msg_t));

	if(status != ESUCCESS) {
		pm_rls_mutex(&mount_point_lock);
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_hp_free(p_mount_point, NULL);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	p_msg->message = MSG_MOUNT;
	p_msg->flags.flags = MFLAG_PMO;
	p_msg->buf.pmo_addr = p_pmo;

	p_info->flags = flags;
	p_info->path_offset = 0;
	p_info->args_offset = rtl_strlen(target) + 1;

	rtl_strcpy_s(((char*) & (p_info->data)) + p_info->path_offset,
	             rtl_strlen(src) + 1,
	             src);
	rtl_strcpy_s(((char*) & (p_info->data)) + p_info->args_offset,
	             rtl_strlen(args) + 1,
	             args);

	//Send message
	status = vfs_send_dev_message(kernel_drv_num,
	                              p_dev->device_number,
	                              p_msg,
	                              &send_result,
	                              &complete_result);

	if(status != ESUCCESS) {
		pm_rls_mutex(&mount_point_lock);
		mm_pmo_unmap(p_info, p_pmo);
		mm_hp_free(p_mount_point, NULL);
		mm_pmo_free(p_pmo);
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(status);
		return status;
	}

	if(send_result != MSTATUS_COMPLETE) {
		pm_rls_mutex(&mount_point_lock);
		mm_pmo_unmap(p_info, p_pmo);
		mm_hp_free(p_mount_point, NULL);
		mm_pmo_free(p_pmo);
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(EACCES);
		return EACCES;
	}

	if(complete_result != ESUCCESS) {
		pm_rls_mutex(&mount_point_lock);
		mm_pmo_unmap(p_info, p_pmo);
		mm_hp_free(p_mount_point, NULL);
		mm_pmo_free(p_pmo);
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(complete_result);
		return complete_result;
	}

	//Create mount point
	p_mount_point->path.p_mount_point = k_path.p_mount_point;
	p_mount_point->path.path = k_path.path;
	p_mount_point->mode = p_info->mode;
	p_mount_point->volume_dev = p_info->volume_dev;
	p_mount_point->gid = gid;
	p_mount_point->uid = uid;
	p_mount_point->fs_dev = p_dev->device_number;
	p_mount_point->mount_points = NULL;

	//Add mount point
	p_parent_point = k_path.p_mount_point;
	rtl_list_insert_after(&(p_parent_point->mount_points),
	                      NULL,
	                      p_mount_point,
	                      NULL);
	p_mount_point->p_parent = p_parent_point;
	pm_rls_mutex(&mount_point_lock);

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

k_status vfs_umount(char* path)
{
	path_t k_path;
	k_status status;
	plist_node_t p_node;
	u32 send_result;
	pmount_point_t p_mount_point;
	pmsg_t p_msg;
	pmsg_umount_info_t p_info;
	k_status complete_result;

	//Get mount point
	pm_acqr_mutex(&mount_point_lock, TIMEOUT_BLOCK);
	status = analyse_path(path, &k_path);

	if(status != ESUCCESS) {
		return status;
	}

	p_mount_point = k_path.p_mount_point;

	if(p_mount_point == NULL) {
		pm_rls_mutex(&mount_point_lock);
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	if(p_mount_point->mount_points != NULL) {
		pm_rls_mutex(&mount_point_lock);
		pm_set_errno(EBUSY);
		return EBUSY;
	}

	//Create message
	status = msg_create(&p_msg, sizeof(msg_t)
	                    + sizeof(msg_umount_info_t));

	if(status != ESUCCESS) {
		pm_rls_mutex(&mount_point_lock);
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(status);
		return status;
	}

	p_msg->message = MSG_UMOUNT;
	p_msg->flags.flags = MFLAG_DIRECTBUF;

	p_info = (pmsg_umount_info_t)(p_msg + 1);
	p_msg->buf.addr = p_info;

	mm_hp_free(k_path.path, NULL);

	//Send message
	status = vfs_send_dev_message(kernel_drv_num,
	                              k_path.p_mount_point->volume_dev,
	                              p_msg,
	                              &send_result,
	                              &complete_result);

	if(status != ESUCCESS) {
		pm_rls_mutex(&mount_point_lock);
		return status;
	}

	if(send_result != MSTATUS_COMPLETE) {
		pm_rls_mutex(&mount_point_lock);
		pm_set_errno(EACCES);
		return EACCES;
	}

	if(complete_result != ESUCCESS) {
		pm_rls_mutex(&mount_point_lock);
		pm_set_errno(complete_result);
		return complete_result;
	}

	//Umount volume
	p_node = rtl_list_get_node_by_item(p_mount_point->p_parent->mount_points,
	                                   p_mount_point);

	rtl_list_remove(&(p_mount_point->p_parent->mount_points),
	                p_node,
	                NULL);
	pm_rls_mutex(&mount_point_lock);

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

//Path
k_status vfs_chroot(char* path)
{
	k_status status;
	pvfs_proc_info p_proc_fd_info;
	path_t k_path;

	//Check if the path exists
	status = vfs_access(path, F_OK);

	if(status != ESUCCESS) {
		pm_set_errno(status);
		return status;
	}

	//Change root directory
	p_proc_fd_info = get_proc_fs_info();
	pm_acqr_mutex(&mount_point_lock, TIMEOUT_BLOCK);
	status = analyse_path(path, &k_path);
	pm_rls_mutex(&mount_point_lock);

	if(status != ESUCCESS) {
		return status;
	}

	pm_acqr_mutex(&(p_proc_fd_info->lock), TIMEOUT_BLOCK);
	mm_hp_free(p_proc_fd_info->root.path, NULL);
	p_proc_fd_info->root.p_mount_point = k_path.p_mount_point;
	p_proc_fd_info->root.path = k_path.path;
	pm_rls_mutex(&(p_proc_fd_info->lock));

	vfs_chdir("/");

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

k_status vfs_chdir(char* path)
{
	k_status status;
	pvfs_proc_info p_proc_fd_info;
	path_t k_path;

	//Check if the path exists
	status = vfs_access(path, F_OK);

	if(status != ESUCCESS) {
		pm_set_errno(status);
		return status;
	}

	//Change work directory
	p_proc_fd_info = get_proc_fs_info();
	pm_acqr_mutex(&mount_point_lock, TIMEOUT_BLOCK);
	status = analyse_path(path, &k_path);
	pm_rls_mutex(&mount_point_lock);

	if(status != ESUCCESS) {
		return status;
	}

	pm_acqr_mutex(&(p_proc_fd_info->lock), TIMEOUT_BLOCK);
	mm_hp_free(p_proc_fd_info->pwd.path, NULL);
	p_proc_fd_info->pwd.p_mount_point = k_path.p_mount_point;
	p_proc_fd_info->pwd.path = k_path.path;
	pm_rls_mutex(&(p_proc_fd_info->lock));

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

k_status vfs_getcwd(char* buf, size_t size)
{
	k_status status;

	pm_acqr_mutex(&mount_point_lock, TIMEOUT_BLOCK);
	status = get_cwd(buf, size);
	pm_rls_mutex(&mount_point_lock);

	pm_set_errno(status);
	return status;
}

//File descriptors
k_status vfs_fork(u32 dest_process)
{
	pvfs_proc_info p_src_info, p_dest_info;
	u32 i;
	pfile_desc_t p_src_fd, p_dest_fd;
	pkobject_t p_obj;

	p_src_info = get_proc_fs_info();

	//Allocate memory for new info
	p_dest_info = mm_hp_alloc(sizeof(vfs_proc_info), NULL);

	if(p_dest_info == NULL) {
		pm_set_errno(ENOMEM);
		return ENOMEM;
	}

	rtl_memcpy(p_dest_info, p_src_info, sizeof(vfs_proc_info));
	vfs_inc_obj_reference((pkobject_t)get_driver(p_src_info->driver_obj));
	pm_init_mutex(&(p_dest_info->lock));

	pm_set_errno(ESUCCESS);

	pm_acqr_mutex(&(p_src_info->lock), TIMEOUT_BLOCK);
	rtl_array_list_init(&(p_dest_info->file_descs),
	                    p_src_info->file_descs.size,
	                    NULL);

	for(i = 0;
	    OPERATE_SUCCESS;
	    i = rtl_array_list_get_next_index(&(p_src_info->file_descs), i + 1)) {

		//Get source file descriptor
		p_src_fd = rtl_array_list_get(&(p_src_info->file_descs), i);
		ASSERT(p_src_fd != NULL);

		//Duplicate file descriptor
		p_dest_fd = mm_hp_alloc(sizeof(file_desc_t), NULL);
		ASSERT(p_dest_fd != NULL);
		rtl_memcpy(p_dest_fd, p_src_fd, sizeof(file_desc_t));
		vfs_inc_obj_reference((pkobject_t)(p_dest_fd->file_obj));

		rtl_array_list_set(&(p_dest_info->file_descs), i, p_dest_fd, NULL);
		ASSERT(OPERATE_SUCCESS);
	}

	rtl_array_list_init(&(p_dest_info->ref_objs),
	                    p_src_info->ref_objs.size,
	                    NULL);

	for(i = 0;
	    OPERATE_SUCCESS;
	    i = rtl_array_list_get_next_index(&(p_src_info->ref_objs), i + 1)) {

		//Get source file descriptor
		p_obj = rtl_array_list_get(&(p_src_info->ref_objs), i);
		ASSERT(p_obj != NULL);

		vfs_inc_obj_reference(p_obj);

		rtl_array_list_set(&(p_dest_info->ref_objs), i, p_obj, NULL);
		ASSERT(OPERATE_SUCCESS);
	}
	pm_rls_mutex(&(p_src_info->lock));

	//Add new info
	set_proc_fs_info(dest_process, p_dest_info);

	return ESUCCESS;
}

void vfs_clean(u32 process_id)
{
	pvfs_proc_info p_info;

	//Get info
	p_info = get_proc_fs_info(process_id);

	if(p_info == NULL) {
		pm_rls_mutex(&file_desc_info_table_lock);
		pm_set_errno(EFAULT);
		return;
	}

	if(pm_is_driver()) {
		io_int_msg_clean(ALL_INTERRUPT);
		vfs_dec_obj_reference((pkobject_t)get_driver(p_info->driver_obj));
	}

	pm_acqr_mutex(&file_desc_info_table_lock, TIMEOUT_BLOCK);

	rtl_array_list_release(&file_desc_info_table,
	                       process_id,
	                       NULL);
	pm_rls_mutex(&file_desc_info_table_lock);

	//Release info
	rtl_array_list_destroy(&(p_info->file_descs),
	                       (item_destroyer_callback)file_desc_destroy_callback,
	                       NULL,
	                       NULL);
	rtl_array_list_destroy(&(p_info->ref_objs),
	                       (item_destroyer_callback)vfs_dec_obj_reference,
	                       NULL,
	                       NULL);
	pm_set_errno(ESUCCESS);
	return;
}

//Files
u32 vfs_open(char* path, u32 flags, u32 mode)
{
	path_t k_path;
	pmsg_t p_msg;
	k_status status, complete_result;
	ppmo_t p_pmo;
	pmsg_open_info_t p_info;
	u32 send_result;
	pfile_desc_t p_fd;
	pvfs_proc_info p_proc_fd_info;
	u32 ret;
	pfile_obj_t p_file_obj;
	pfile_obj_ref_t p_ref;

	//Analyse path
	pm_acqr_mutex(&mount_point_lock, TIMEOUT_BLOCK);
	status = analyse_path(path, &k_path);
	pm_rls_mutex(&mount_point_lock);

	if(status != ESUCCESS) {
		return INVALID_FD;
	}

	//Create buffer
	p_pmo = mm_pmo_create(sizeof(pmsg_open_info_t) + PATH_MAX);

	if(p_pmo == NULL) {
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(EFAULT);
		return INVALID_FD;
	}

	p_info = mm_pmo_map(NULL, p_pmo, false);

	if(p_info == NULL) {
		mm_pmo_free(p_pmo);
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(EFAULT);
		return INVALID_FD;
	}

	p_fd = mm_hp_alloc(sizeof(file_desc_t), NULL);

	if(p_fd == NULL) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_hp_free(k_path.path, NULL);
		mm_pmo_free(p_pmo);
		pm_set_errno(EFAULT);
		return INVALID_FD;
	}

	p_ref = mm_hp_alloc(sizeof(file_obj_ref_t), NULL);

	if(p_ref == NULL) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_hp_free(p_fd, NULL);
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(EFAULT);
		return INVALID_FD;
	}

	//Create message
	status = msg_create(&p_msg,
	                    sizeof(msg_t));

	if(status != ESUCCESS) {
		mm_hp_free(p_ref, NULL);
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_hp_free(p_fd, NULL);
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(EFAULT);
		return INVALID_FD;
	}

	p_msg->message = MSG_OPEN;
	p_msg->flags.flags = MFLAG_PMO;
	p_msg->buf.pmo_addr = p_pmo;

	p_info->process = pm_get_crrnt_process();
	p_info->mode = mode;
	p_info->flags = flags;

	rtl_strcpy_s(&(p_info->path), PATH_MAX, k_path.path);

	//Send message
	status = vfs_send_dev_message(kernel_drv_num,
	                              k_path.p_mount_point->volume_dev,
	                              p_msg,
	                              &send_result,
	                              &complete_result);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_hp_free(k_path.path, NULL);
		mm_hp_free(p_fd, NULL);
		mm_hp_free(p_ref, NULL);
		pm_set_errno(status);
		return INVALID_FD;
	}

	if(send_result != MSTATUS_COMPLETE) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_hp_free(p_fd, NULL);
		mm_hp_free(p_ref, NULL);
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(EACCES);
		return INVALID_FD;
	}

	if(complete_result != ESUCCESS) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_hp_free(p_fd, NULL);
		mm_hp_free(k_path.path, NULL);
		mm_hp_free(p_ref, NULL);
		pm_set_errno(complete_result);
		return INVALID_FD;
	}

	//Create file descriptor
	p_fd->file_obj = p_info->file_object;
	p_fd->offset = 0;
	p_file_obj = get_file_obj(p_fd->file_obj);
	p_file_obj->size = p_info->file_size;
	p_fd->flags = flags;
	p_fd->serial_read = p_info->serial_read;
	rtl_memcpy(&(p_fd->path), &k_path, sizeof(path_t));

	//Add file descriptor
	p_proc_fd_info = get_proc_fs_info();

	pm_acqr_mutex(&(p_proc_fd_info->lock), TIMEOUT_BLOCK);

	ret = rtl_array_list_get_free_index(&(p_proc_fd_info->file_descs));
	rtl_array_list_set(&(p_proc_fd_info->file_descs), ret, p_fd, NULL);
	ASSERT(OPERATE_SUCCESS);

	pm_rls_mutex(&(p_proc_fd_info->lock));

	mm_pmo_unmap(p_info, p_pmo);
	mm_pmo_free(p_pmo);

	vfs_inc_obj_reference((pkobject_t)p_file_obj);

	//Add process
	p_ref->fd = ret;
	p_ref->process_id = pm_get_crrnt_process();

	pm_acqr_mutex(&(p_file_obj->refered_proc_list_lock), TIMEOUT_BLOCK);
	rtl_list_insert_after(&(p_file_obj->refered_proc_list),
	                      NULL,
	                      p_ref,
	                      NULL);
	pm_rls_mutex(&(p_file_obj->refered_proc_list_lock));
	mm_hp_free(k_path.path, NULL);

	pm_set_errno(ESUCCESS);

	return ret;
}

k_status vfs_chmod(u32 fd, u32 mode)
{
	pfile_desc_t p_fd;
	pmsg_t p_msg;
	k_status status, complete_result;
	u32 send_result;
	pmsg_chmod_info_t p_info;

	//Get file descriptor
	p_fd = get_file_descriptor(fd);

	if(p_fd == NULL) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	//Create message
	status = msg_create(&p_msg, sizeof(msg_t) + sizeof(msg_chmod_info_t));

	if(status != ESUCCESS) {
		return status;
	}

	p_msg->message = MSG_CHOMD;
	p_msg->flags.flags = MFLAG_DIRECTBUF;
	p_msg->buf.addr = (pmsg_chmod_info_t)(p_msg + 1);

	p_info = p_msg->buf.addr;
	p_info->process = pm_get_crrnt_process();
	p_info->mode = mode;

	//Send message
	status = vfs_send_dev_message(kernel_drv_num,
	                              p_fd->file_obj,
	                              p_msg,
	                              &send_result,
	                              &complete_result);

	if(status != ESUCCESS) {
		return status;
	}

	if(send_result != MSTATUS_COMPLETE) {
		pm_set_errno(EACCES);
		return EACCES;
	}

	pm_set_errno(complete_result);
	return complete_result;
}

k_status vfs_access(char* path, u32 mode)
{
	path_t path_info;
	pmsg_t p_msg;
	k_status status, complete_result;
	u32 send_result;
	pmsg_access_info_t p_info;
	size_t len;

	pm_acqr_mutex(&mount_point_lock, TIMEOUT_BLOCK);
	status = analyse_path(path, &path_info);
	pm_rls_mutex(&mount_point_lock);

	if(!OPERATE_SUCCESS) {
		return EFAULT;
	}

	len = rtl_strlen(path_info.path);

	//Create message
	status = msg_create(&p_msg, sizeof(msg_t)
	                    + sizeof(msg_access_info_t)
	                    + len);

	if(status != ESUCCESS) {
		mm_hp_free(path_info.path, NULL);
		pm_set_errno(status);
		return status;
	}

	p_msg->message = MSG_ACCESS;
	p_msg->flags.flags = MFLAG_DIRECTBUF;

	p_info = (pmsg_access_info_t)(p_msg + 1);
	p_msg->buf.addr = p_info;

	p_info->mode = mode;
	p_info->process = pm_get_crrnt_process();

	rtl_strcpy_s(&(p_info->path), len + 1, path_info.path);
	mm_hp_free(path_info.path, NULL);

	//Send message
	status = vfs_send_dev_message(kernel_drv_num,
	                              path_info.p_mount_point->volume_dev,
	                              p_msg,
	                              &send_result,
	                              &complete_result);

	if(status != ESUCCESS) {
		return status;
	}

	if(send_result != MSTATUS_COMPLETE) {
		pm_set_errno(EACCES);
		return EACCES;
	}

	pm_set_errno(complete_result);
	return complete_result;
}

void vfs_close(u32 fd)
{
	pvfs_proc_info p_info;
	pfile_desc_t p_desc;

	p_info = get_proc_fs_info();

	//Get file descriptor
	pm_acqr_mutex(&(p_info->lock), TIMEOUT_BLOCK);

	p_desc = rtl_array_list_get(&(p_info->file_descs), fd);

	if(p_desc == NULL) {
		pm_rls_mutex(&(p_info->lock));

		pm_set_errno(ESUCCESS);
		return;
	}

	//Release file descriptor
	rtl_array_list_release(&(p_info->file_descs), fd, NULL);
	pm_rls_mutex(&(p_info->lock));

	//Decrease reference count of file object
	vfs_dec_obj_reference((pkobject_t)get_file_obj(p_desc->file_obj));

	if(p_desc->path.p_mount_point != NULL) {
		mm_hp_free(p_desc->path.path, NULL);
	}

	mm_hp_free(p_desc, NULL);

	pm_set_errno(ESUCCESS);
	return;
}

k_status vfs_read(u32 fd, ppmo_t buf)
{
	pfile_desc_t p_fd;
	pmsg_t p_msg;
	k_status status;
	pmsg_read_info_t p_info;
	pmsg_read_data_t p_data;
	k_status complete_state;
	u32 msg_state;

	//Get file descriptor
	p_fd = get_file_descriptor(fd);

	if(p_fd->flags | O_WRONLY) {
		pm_set_errno(EBADF);
		return EBADF;
	}

	if(p_fd->flags | O_DIRECTORY) {
		pm_set_errno(EISDIR);
		return EISDIR;
	}

	//Map buffer
	p_info = mm_pmo_map(NULL, buf, false);

	if(p_info == NULL) {
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	//Check buffer size
	if(buf->size < p_info->len + sizeof(msg_read_data_t) - sizeof(u8)) {
		mm_pmo_unmap(p_info, buf);
		pm_set_errno(EOVERFLOW);
		return EOVERFLOW;
	}

	//Create message
	status = msg_create(&p_msg, sizeof(msg_t));

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, buf);
		return status;
	}

	p_msg->message = MSG_READ;
	p_msg->flags.flags = MFLAG_PMO;
	p_msg->buf.pmo_addr = buf;

	p_info->file_obj = p_fd->file_obj;
	p_info->offset = p_fd->offset;

	//Send message
	status = vfs_send_file_message(kernel_drv_num,
	                               p_fd->file_obj,
	                               p_msg,
	                               &msg_state,
	                               &complete_state);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, buf);
		pm_set_errno(status);
		return status;
	}

	if(msg_state != MSTATUS_COMPLETE) {
		mm_pmo_unmap(p_info, buf);
		pm_set_errno(EACCES);
		return EACCES;
	}

	p_data = (pmsg_read_data_t)p_info;

	if(!p_fd->serial_read) {
		//Move file pointer
		(p_fd->offset) += p_data->len;
	}

	mm_pmo_unmap(p_info, buf);
	pm_set_errno(complete_state);
	return complete_state;
}

size_t vfs_write(u32 fd, ppmo_t buf)
{
	pfile_desc_t p_fd;
	pmsg_t p_msg;
	k_status status;
	pmsg_write_info_t p_info;
	k_status complete_state;
	u32 msg_state;
	pfile_obj_t p_fo;

	//Get file descriptor
	p_fd = get_file_descriptor(fd);

	if(!((p_fd->flags | O_WRONLY) || (p_fd->flags | O_RDWR))) {
		pm_set_errno(EBADF);
		return EBADF;
	}

	if(p_fd->flags | O_DIRECTORY) {
		pm_set_errno(EISDIR);
		return EISDIR;
	}

	//Map buffer
	p_info = mm_pmo_map(NULL, buf, false);

	if(p_info == NULL) {
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	//Check buffer size
	if(buf->size < p_info->len + sizeof(msg_write_info_t) - sizeof(u8)) {
		mm_pmo_unmap(p_info, buf);
		pm_set_errno(EOVERFLOW);
		return EOVERFLOW;
	}

	//Create message
	status = msg_create(&p_msg, sizeof(msg_t));

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, buf);
		return status;
	}

	p_msg->message = MSG_WRITE;
	p_msg->flags.flags = MFLAG_PMO;
	p_msg->buf.pmo_addr = buf;

	p_info->file_obj = p_fd->file_obj;
	p_info->offset = p_fd->offset;

	//Send message
	status = vfs_send_file_message(kernel_drv_num,
	                               p_fd->file_obj,
	                               p_msg,
	                               &msg_state,
	                               &complete_state);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, buf);
		pm_set_errno(status);
		return status;
	}

	if(msg_state != MSTATUS_COMPLETE) {
		mm_pmo_unmap(p_info, buf);
		pm_set_errno(EACCES);
		return EACCES;
	}

	if(!p_fd->serial_read) {
		//Move file pointer
		p_fo = get_file_obj(p_fd->file_obj);
		(p_fd->offset) += p_info->len;

		if(p_fd->offset > p_fo->size) {
			//Reset file size
			p_fo->size = p_fd->offset;
		}
	}

	mm_pmo_unmap(p_info, buf);
	pm_set_errno(complete_state);
	return complete_state;

}

u64 vfs_seek(u32 fd, u32 pos, s64 offset)
{
	pfile_desc_t p_fd;
	pfile_obj_t p_fo;

	//Get file descriptor
	p_fd = get_file_descriptor(fd);

	if(p_fd == NULL) {
		pm_set_errno(EBADF);
		return 0;
	}

	if(p_fd->serial_read) {
		pm_set_errno(ESPIPE);
		return 0;
	}

	p_fo = get_file_obj(p_fd->file_obj);

	//Get start pos
	switch(pos) {
	case SEEK_SET:
		p_fd->offset = 0;
		break;

	case SEEK_CUR:
		break;

	case SEEK_END:
		p_fd->offset = p_fo->size;
		break;

	default:
		pm_set_errno(EINVAL);
		return 0;
	}

	//Add offset
	if(offset > 0) {
		if(offset > p_fo->size - p_fd->offset) {
			offset = p_fo->size - p_fd->offset;
		}

	} else {
		if((-offset) > p_fd->offset) {
			offset = (-(s64)(p_fd->offset));
		}
	}

	(p_fd->offset) += offset;

	pm_set_errno(ESUCCESS);
	return p_fd->offset;
}

k_status vfs_stat(char* path, ppmo_t buf)
{
	pmsg_t p_msg;
	k_status status;
	pmsg_stat_info_t p_info;
	k_status complete_state;
	u32 msg_state;
	path_t k_path;

	pm_acqr_mutex(&mount_point_lock, TIMEOUT_BLOCK);
	status = analyse_path(path, &k_path);
	pm_rls_mutex(&mount_point_lock);

	if(!OPERATE_SUCCESS) {
		return EFAULT;
	}

	//Map buffer
	p_info = mm_pmo_map(NULL, buf, false);

	if(p_info == NULL) {
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	//Check buffer size
	if(buf->size < sizeof(file_stat_t)
	   || buf->size < rtl_strlen(path) + 1) {
		mm_pmo_unmap(p_info, buf);
		mm_hp_free(k_path.path, NULL);
		pm_set_errno(EOVERFLOW);
		return EOVERFLOW;
	}

	//Create message
	status = msg_create(&p_msg, sizeof(msg_t));

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, buf);
		mm_hp_free(k_path.path, NULL);
		return status;
	}

	p_msg->message = MSG_STAT;
	p_msg->flags.flags = MFLAG_PMO;
	p_msg->buf.pmo_addr = buf;

	rtl_strcpy_s(&(p_info->path), buf->size, k_path.path);

	//Send message
	status = vfs_send_file_message(kernel_drv_num,
	                               k_path.p_mount_point->volume_dev,
	                               p_msg,
	                               &msg_state,
	                               &complete_state);

	mm_hp_free(k_path.path, NULL);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, buf);
		pm_set_errno(status);
		return status;
	}

	if(msg_state != MSTATUS_COMPLETE) {
		mm_pmo_unmap(p_info, buf);
		pm_set_errno(EACCES);
		return EACCES;
	}

	mm_pmo_unmap(p_info, buf);
	pm_set_errno(complete_state);
	return complete_state;
}

k_status vfs_unlink(char* path)
{
	path_t path_info;
	pmsg_t p_msg;
	k_status status, complete_result;
	u32 send_result;
	pmsg_unlink_info_t p_info;
	size_t len;

	pm_acqr_mutex(&mount_point_lock, TIMEOUT_BLOCK);
	status = analyse_path(path, &path_info);
	pm_rls_mutex(&mount_point_lock);

	if(!OPERATE_SUCCESS) {
		return EFAULT;
	}

	len = rtl_strlen(path_info.path);

	//Create message
	status = msg_create(&p_msg, sizeof(msg_t)
	                    + sizeof(msg_unlink_info_t)
	                    + len);

	if(status != ESUCCESS) {
		mm_hp_free(path_info.path, NULL);
		pm_set_errno(status);
		return status;
	}

	p_msg->message = MSG_UNLINK;
	p_msg->flags.flags = MFLAG_DIRECTBUF;

	p_info = (pmsg_unlink_info_t)(p_msg + 1);
	p_msg->buf.addr = p_info;

	p_info->process = pm_get_crrnt_process();

	rtl_strcpy_s(&(p_info->path), len + 1, path_info.path);
	mm_hp_free(path_info.path, NULL);

	//Send message
	status = vfs_send_dev_message(kernel_drv_num,
	                              path_info.p_mount_point->volume_dev,
	                              p_msg,
	                              &send_result,
	                              &complete_result);

	if(status != ESUCCESS) {
		return status;
	}

	if(send_result != MSTATUS_COMPLETE) {
		pm_set_errno(EACCES);
		return EACCES;
	}

	pm_set_errno(complete_result);
	return complete_result;

}

k_status vfs_mkdir(char* path, u32 mode)
{
	path_t path_info;
	pmsg_t p_msg;
	k_status status, complete_result;
	u32 send_result;
	pmsg_mkdir_info_t p_info;
	size_t len;

	pm_acqr_mutex(&mount_point_lock, TIMEOUT_BLOCK);
	status = analyse_path(path, &path_info);
	pm_rls_mutex(&mount_point_lock);

	if(!OPERATE_SUCCESS) {
		return EFAULT;
	}

	len = rtl_strlen(path_info.path);

	//Create message
	status = msg_create(&p_msg, sizeof(msg_t)
	                    + sizeof(msg_mkdir_info_t)
	                    + len);

	if(status != ESUCCESS) {
		mm_hp_free(path_info.path, NULL);
		pm_set_errno(status);
		return status;
	}

	p_msg->message = MSG_MKDIR;
	p_msg->flags.flags = MFLAG_DIRECTBUF;

	p_info = (pmsg_mkdir_info_t)(p_msg + 1);
	p_msg->buf.addr = p_info;

	p_info->mode = mode;
	p_info->process = pm_get_crrnt_process();

	rtl_strcpy_s(&(p_info->path), len + 1, path_info.path);
	mm_hp_free(path_info.path, NULL);

	//Send message
	status = vfs_send_dev_message(kernel_drv_num,
	                              path_info.p_mount_point->volume_dev,
	                              p_msg,
	                              &send_result,
	                              &complete_result);

	if(status != ESUCCESS) {
		return status;
	}

	if(send_result != MSTATUS_COMPLETE) {
		pm_set_errno(EACCES);
		return EACCES;
	}

	pm_set_errno(complete_result);
	return complete_result;
}

k_status vfs_readdir(u32 fd, ppmo_t buf)
{
	pfile_desc_t p_fd;
	pmsg_t p_msg;
	k_status status;
	pmsg_readdir_info_t p_info;
	k_status complete_state;
	u32 msg_state;

	//Get file descriptor
	p_fd = get_file_descriptor(fd);

	if(p_fd->flags | O_WRONLY) {
		pm_set_errno(EBADF);
		return EBADF;
	}

	if(!(p_fd->flags | O_DIRECTORY)) {
		pm_set_errno(ENOTDIR);
		return ENOTDIR;
	}

	//Map buffer
	p_info = mm_pmo_map(NULL, buf, false);

	if(p_info == NULL) {
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	//Check buffer size
	if(buf->size < p_info->count * sizeof(dirent_t)
	   + sizeof(msg_readdir_info_t)
	   - sizeof(dirent_t)) {
		mm_pmo_unmap(p_info, buf);
		pm_set_errno(EOVERFLOW);
		return EOVERFLOW;
	}

	//Create message
	status = msg_create(&p_msg, sizeof(msg_t));

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, buf);
		return status;
	}

	p_msg->message = MSG_READDIR;
	p_msg->flags.flags = MFLAG_PMO;
	p_msg->buf.pmo_addr = buf;

	p_info->file_obj = p_fd->file_obj;
	p_info->offset = p_fd->offset;

	//Send message
	status = vfs_send_file_message(kernel_drv_num,
	                               p_fd->file_obj,
	                               p_msg,
	                               &msg_state,
	                               &complete_state);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, buf);
		pm_set_errno(status);
		return status;
	}

	if(msg_state != MSTATUS_COMPLETE) {
		mm_pmo_unmap(p_info, buf);
		pm_set_errno(EACCES);
		return EACCES;
	}

	p_fd->offset += p_info->count;

	mm_pmo_unmap(p_info, buf);
	pm_set_errno(complete_state);
	return complete_state;
}

//s32			vfs_ioctl(u32 fd, u32 request, ...);

//File object
u32 vfs_create_file_object()
{
	pfile_obj_t p_file_obj;

	//Allocate memory
	p_file_obj = mm_hp_alloc(sizeof(file_obj_t), NULL);

	if(p_file_obj == NULL) {
		pm_set_errno(EFAULT);
		return INVALID_FILEID;
	}


	//Initialize object
	vfs_initialize_object((pkobject_t)p_file_obj);

	p_file_obj->p_driver = get_driver(get_proc_fs_info()->driver_obj);
	p_file_obj->refered_proc_list = NULL;
	pm_init_mutex(&(p_file_obj->refered_proc_list_lock));

	//Regist object
	if(add_file_obj(p_file_obj) != ESUCCESS) {
		mm_hp_free(p_file_obj, NULL);
		return INVALID_FILEID;
	}

	p_file_obj->obj.ref_count = 0;
	p_file_obj->obj.destroy_callback = (obj_destroyer)(&file_objecj_release_callback);

	pm_set_errno(ESUCCESS);
	return p_file_obj->file_id;
}

k_status vfs_send_file_message(u32 src_driver,
                               u32 dest_file,
                               pmsg_t p_msg,
                               u32* p_result,
                               k_status* p_complete_result)
{
	pdriver_obj_t p_src_drv;
	pfile_obj_t p_dest_file;

	p_src_drv = get_driver(src_driver);

	if(!OPERATE_SUCCESS) {
		return pm_get_errno();
	}

	p_dest_file = get_file_obj(dest_file);

	if(!OPERATE_SUCCESS) {
		return pm_get_errno();
	}

	p_msg->file_id = dest_file;
	p_msg->src_thread = pm_get_crrnt_thrd_id();
	p_msg->result_queue = p_src_drv->msg_queue;

	return msg_send(p_msg,
	                p_dest_file->p_driver->msg_queue,
	                p_result,
	                p_complete_result);
}

u32 vfs_get_crrnt_driver_id()
{
	pvfs_proc_info p_info;
	p_info = get_proc_fs_info();
	return p_info->driver_obj;
}

u32 vfs_add_proc_obj(pkobject_t p_object)
{
	pvfs_proc_info p_info;
	u32 index;
	k_status status;

	p_info = get_proc_fs_info();
	
	pm_acqr_mutex(&(p_info->lock),TIMEOUT_BLOCK);
	index=rtl_array_list_get_free_index(&(p_info->ref_objs));
	status=pm_get_errno();
	
	if(status!=ESUCCESS){
		pm_rls_mutex(&(p_info->lock));
		pm_set_errno(status);
		return 0;
	}
	
	status=rtl_array_list_set(&(p_info->ref_objs),index,p_object,NULL);
	if(status!=ESUCCESS){
		pm_rls_mutex(&(p_info->lock));
		pm_set_errno(status);
		return 0;
	}
	
	pm_rls_mutex(&(p_info->lock));
	
	vfs_inc_obj_reference(p_object);
	
	pm_set_errno(ESUCCESS);
	return index;
}

void vfs_remove_proc_obj(u32 index)
{
	pvfs_proc_info p_info;
	k_status status;
	pkobject_t p_object;

	p_info = get_proc_fs_info();
	
	pm_acqr_mutex(&(p_info->lock),TIMEOUT_BLOCK);
	
	p_object=rtl_array_list_get(&(p_info->ref_objs), index);
	if(p_object==NULL){
		status=pm_get_errno();
		pm_rls_mutex(&(p_info->lock));
		pm_set_errno(status);
		return;
	}
	rtl_array_list_release(&(p_info->ref_objs), index, NULL);
	status=pm_get_errno();
	if(status!=ESUCCESS){
		pm_rls_mutex(&(p_info->lock));
		pm_set_errno(status);
		return;
	}
	pm_rls_mutex(&(p_info->lock));
	
	vfs_dec_obj_reference(p_object);
	
	pm_set_errno(ESUCCESS);
	return;
}

k_status add_file_obj(pfile_obj_t p_file_obj)
{
	u32 index;
	k_status status;

	pm_acqr_mutex(&(file_obj_table_lock), TIMEOUT_BLOCK);

	index = rtl_array_list_get_free_index(&file_obj_table);

	if(!OPERATE_SUCCESS) {
		status = pm_get_errno();
		pm_rls_mutex(&(file_obj_table_lock));
		pm_set_errno(status);
		return status;
	}

	rtl_array_list_set(&file_obj_table, index, p_file_obj, NULL);

	if(!OPERATE_SUCCESS) {
		status = pm_get_errno();
		pm_rls_mutex(&(file_obj_table_lock));
		pm_set_errno(status);
		return status;
	}

	p_file_obj->file_id = index;

	pm_rls_mutex(&(file_obj_table_lock));

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

void remove_file_obj(pfile_obj_t p_file_obj)
{
	//Release file id
	vfs_inc_obj_reference((pkobject_t)p_file_obj);

	pm_acqr_mutex(&file_obj_table_lock, TIMEOUT_BLOCK);

	rtl_array_list_release(&file_obj_table, p_file_obj->file_id, NULL);

	pm_rls_mutex(&file_obj_table_lock);

	pm_acqr_mutex(&(p_file_obj->refered_proc_list_lock), TIMEOUT_BLOCK);

	rtl_list_destroy(&(p_file_obj->refered_proc_list),
	                 NULL,
	                 (item_destroyer_callback)ref_proc_destroy_callback,
	                 p_file_obj);

	pm_rls_mutex(&(p_file_obj->refered_proc_list_lock));

	vfs_dec_obj_reference((pkobject_t)p_file_obj);
	return;
}

pvfs_proc_info get_proc_fs_info()
{
	pvfs_proc_info ret;

	pm_acqr_mutex(&file_desc_info_table_lock, TIMEOUT_BLOCK);

	ret = rtl_array_list_get(&file_desc_info_table, pm_get_crrnt_process());

	if(ret == NULL) {
		excpt_panic(EFAULT,
		            "Current process doesn't have a file descriptor table.It seems impossible but it happend.It's properly because of buffer overflow.");
	}

	pm_rls_mutex(&file_desc_info_table_lock);
	return ret;
}

void set_proc_fs_info(u32 process_id,
                      pvfs_proc_info p_new_info)
{
	pm_acqr_mutex(&file_desc_info_table_lock, TIMEOUT_BLOCK);

	ASSERT((rtl_array_list_get(&file_desc_info_table, process_id),
	        !OPERATE_SUCCESS));

	rtl_array_list_set(&file_desc_info_table, process_id, p_new_info, NULL);

	pm_rls_mutex(&file_desc_info_table_lock);

	return;
}

void set_drv_obj(u32 driver_id)
{
	pvfs_proc_info p_info;

	p_info = get_proc_fs_info();

	p_info->driver_obj = driver_id;

	return;
}

bool has_drv_object()
{
	pvfs_proc_info p_info;

	p_info = get_proc_fs_info();

	if(p_info->driver_obj != INVALID_DRV) {
		return false;
	}

	return true;
}

void ref_proc_destroy_callback(pfile_obj_ref_t p_item,
                               pfile_obj_t p_file_object)
{
	pvfs_proc_info p_info;
	pfile_desc_t p_fd;

	//Release file descriptor
	pm_acqr_mutex(&file_desc_info_table_lock, TIMEOUT_BLOCK);
	p_info = rtl_array_list_get(&file_desc_info_table, p_item->process_id);

	if(p_info == NULL) {
		excpt_panic(EFAULT,
		            "VFS data structure broken!");
	}

	pm_rls_mutex(&file_desc_info_table_lock);

	pm_acqr_mutex(&(p_info->lock), TIMEOUT_BLOCK);

	p_fd = rtl_array_list_get(&(p_info->file_descs), p_item->fd);

	if(p_fd == NULL) {
		excpt_panic(EFAULT,
		            "VFS data structure broken!");
	}

	mm_hp_free(p_fd, NULL);

	rtl_array_list_release(&(p_info->file_descs), p_item->fd, NULL);

	pm_rls_mutex(&(p_info->lock));

	mm_hp_free(p_item, NULL);

	//Decrease reference counnt
	vfs_dec_obj_reference((pkobject_t)p_file_object);

	return;
}

pfile_obj_t get_file_obj(u32 id)
{
	pfile_obj_t ret;
	k_status status;

	pm_acqr_mutex(&file_obj_table_lock, TIMEOUT_BLOCK);

	ret = rtl_array_list_get(&file_desc_info_table, id);
	status = pm_get_errno();

	pm_rls_mutex(&file_obj_table_lock);

	pm_set_errno(status);

	return ret;
}

void file_desc_destroy_callback(pfile_desc_t p_fd,
                                void* p_null)
{
	pkobject_t p_file_obj;

	p_file_obj = (pkobject_t)get_file_obj(p_fd->file_obj);
	vfs_dec_obj_reference(p_file_obj);
	mm_hp_free(p_fd, NULL);

	UNREFERRED_PARAMETER(p_null);
	return;
}

k_status analyse_path(char* path, ppath_t ret)
{
	char* p;
	char* name_buf;
	char* path_buf;
	char* p_name;
	pmount_point_t p_mount_point, p_current_mount_point;;
	stack_t path_stack;
	size_t len;
	size_t path_buf_len;
	plist_node_t p_node;
	k_status status;
	pvfs_proc_info p_info;

	p_info = get_proc_fs_info();
	path_buf_len = rtl_strlen(path) + 1 + PATH_MAX
	               + rtl_strlen(p_info->root.path);

	path_stack = NULL;

	if(rtl_strlen(path) > PATH_MAX) {
		pm_set_errno(ENAMETOOLONG);
		return ENAMETOOLONG;
	}

	//Allocate memory
	name_buf = mm_hp_alloc(NAME_MAX + 1, NULL);

	if(name_buf == NULL) {
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	path_buf = mm_hp_alloc(path_buf_len, NULL);

	if(path_buf == NULL) {
		mm_hp_free(name_buf, NULL);
		pm_set_errno(EFAULT);
		return EFAULT;
	}

	//Analyse path
	if(*path == '/') {
		//The path begins from root directory
		//Push directories name in the stack
		for(p = path, rtl_get_next_name_in_path(&p, name_buf, NAME_MAX + 1);
		    OPERATE_SUCCESS && *name_buf != '\0';
		    rtl_get_next_name_in_path(&p, name_buf, NAME_MAX + 1)) {
			if(rtl_strcmp(name_buf, ".") == 0) {
				continue;

			} else if(rtl_strcmp(name_buf, "..") == 0) {
				if(path_stack == NULL) {
					mm_hp_free(path_buf, NULL);
					mm_hp_free(name_buf, NULL);
					pm_set_errno(ENOENT);
					return ENOENT;

				} else {
					p_name = rtl_stack_pop(&path_stack, NULL);
					mm_hp_free(p_name, NULL);
				}

			} else {
				len = rtl_strlen(name_buf) + 1;
				p_name = mm_hp_alloc(len, NULL);
				rtl_strcpy_s(p_name, len , name_buf);
				rtl_stack_push(&path_stack, p_name , NULL);
			}
		}

		//Get path
		rtl_strcpy_s(path_buf, path_buf_len, p_info->root.path);

		while(path_stack != NULL) {
			rtl_strcat_s(path_buf, path_buf_len , "/");
			p_name = path_stack->p_item;
			rtl_strcat_s(path_buf, path_buf_len, p_name);
			rtl_list_remove(&path_stack, path_stack, NULL);
		}

	} else {
		//The path begins from current directory
		status = get_cwd(path_buf, path_buf_len);

		if(status != ESUCCESS) {
			return status;
		}

		rtl_strcat_s(path_buf, path_buf_len, "/");
		rtl_strcat_s(path_buf, path_buf_len, path);

		//Push directories name in the stack
		for(p = path_buf, rtl_get_next_name_in_path(&p, name_buf, NAME_MAX + 1);
		    OPERATE_SUCCESS && *name_buf != '\0';
		    rtl_get_next_name_in_path(&p, name_buf, NAME_MAX + 1)) {
			if(rtl_strcmp(name_buf, ".") == 0) {
				continue;

			} else if(rtl_strcmp(name_buf, "..") == 0) {
				if(path_stack == NULL) {
					mm_hp_free(path_buf, NULL);
					mm_hp_free(name_buf, NULL);
					pm_set_errno(ENOENT);
					return ENOENT;

				} else {
					p_name = rtl_stack_pop(&path_stack, NULL);
					mm_hp_free(p_name, NULL);
				}

			} else {
				len = rtl_strlen(name_buf) + 1;
				p_name = mm_hp_alloc(len, NULL);
				rtl_strcpy_s(p_name, len , name_buf);
				rtl_stack_push(&path_stack, p_name , NULL);
			}
		}

		//Get path
		rtl_strcpy_s(path_buf, path_buf_len, p_info->root.path);

		while(path_stack != NULL) {
			rtl_strcat_s(path_buf, path_buf_len , "/");
			p_name = path_stack->p_item;
			rtl_strcat_s(path_buf, path_buf_len, p_name);
			rtl_list_remove(&path_stack, path_stack, NULL);
		}
	}

	//Get mount point
	p = path_buf;
	p++;
	p_current_mount_point = p_info->root.p_mount_point;

	while(p_current_mount_point->mount_points != NULL) {
		p_node = p_current_mount_point->mount_points;

		while(1) {
			p_mount_point = (pmount_point_t)(p_node->p_item);

			if(rtl_is_sub_string(p, p_mount_point->path.path)) {
				p_current_mount_point = p_mount_point;
				p += (rtl_strlen(p_mount_point->path.path) + 1);
				break;
			}

			p_node = p_node->p_next;

			if(p_node == p_current_mount_point->mount_points) {
				goto _MOUNT_POINT_BREAK;
			}
		}
	}

_MOUNT_POINT_BREAK:
	ret->p_mount_point = p_current_mount_point;
	len = rtl_strlen(p) + 1;
	ret->path = mm_hp_alloc(len, NULL);
	rtl_strcpy_s(ret->path, len, p);

	mm_hp_free(path_buf, NULL);
	mm_hp_free(name_buf, NULL);
	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

pfile_desc_t get_file_descriptor(u32 fd)
{
	pfile_desc_t ret;
	pvfs_proc_info p_info;

	p_info = get_proc_fs_info();

	pm_acqr_mutex(&(p_info->lock), TIMEOUT_BLOCK);

	ret = rtl_array_list_get(&(p_info->file_descs), fd);

	pm_rls_mutex(&(p_info->lock));

	return ret;
}

void file_objecj_release_callback(pfile_obj_t p_fo)
{
	//Send message
	send_file_obj_destroy_msg(p_fo);

	//Remove file object
	pm_acqr_mutex(&file_obj_table_lock, TIMEOUT_BLOCK);

	rtl_array_list_release(&file_obj_table, p_fo->file_id, NULL);

	pm_rls_mutex(&file_obj_table_lock);

	pm_acqr_mutex(&(p_fo->refered_proc_list_lock), TIMEOUT_BLOCK);

	rtl_list_destroy(&(p_fo->refered_proc_list),
	                 NULL,
	                 (item_destroyer_callback)ref_proc_destroy_callback,
	                 p_fo);

	pm_rls_mutex(&(p_fo->refered_proc_list_lock));

	//Free memory
	mm_hp_free(p_fo, NULL);

	return;
}

void send_file_obj_destroy_msg(pfile_obj_t p_file_obj)
{
	pmsg_t p_msg;
	pmsg_close_info_t p_info;
	k_status status;

	//Create message
	status = msg_create(&p_msg, sizeof(msg_t) + sizeof(msg_close_info_t));

	if(status != ESUCCESS) {
		return;
	}

	p_msg->message = MSG_CLOSE;
	p_info = (pmsg_close_info_t)(p_msg + 1);
	p_msg->buf.addr = p_info;
	p_info->file_obj_id = p_file_obj->file_id;
	p_msg->flags.flags = MFLAG_DIRECTBUF;

	//Send message
	vfs_send_file_message(kernel_drv_num, p_file_obj->file_id, p_msg, NULL, NULL);

	return;
}

k_status get_cwd(char* buf, size_t size)
{
	pvfs_proc_info p_info;
	stack_t mount_point_stack;
	pmount_point_t p_mount_point;

	p_info = get_proc_fs_info();
	p_mount_point = p_info->pwd.p_mount_point;

	//Push mount points
	while(1) {
		rtl_stack_push(&mount_point_stack, p_mount_point, NULL);

		if(p_mount_point == p_info->root.p_mount_point) {
			break;

		} else {
			p_mount_point = p_mount_point->p_parent;
		}
	}

	//Pop mount points
	*buf = '\0';

	while(mount_point_stack != NULL) {
		rtl_strcat_s(buf, size, "/");
		p_mount_point = rtl_stack_pop(&mount_point_stack, NULL);

		if(p_mount_point == p_info->root.p_mount_point) {
			rtl_strcat_s(buf,
			             size,
			             p_mount_point->path.path
			             + rtl_strlen(p_info->root.path));

		} else {
			rtl_strcat_s(buf, size, p_mount_point->path.path);
		}
	}

	if(*(p_mount_point->path.path) != '\0') {
		rtl_strcat_s(buf, size, "/");
		rtl_strcat_s(buf, size, p_info->pwd.path);
	}

	pm_set_errno(ESUCCESS);
	return ESUCCESS;
}

u32 get_initrd_fd()
{
	pmsg_t p_msg;
	k_status status, complete_result;
	ppmo_t p_pmo;
	pmsg_open_info_t p_info;
	u32 send_result;
	pfile_desc_t p_fd;
	pvfs_proc_info p_proc_fd_info;
	u32 ret;
	pfile_obj_t p_file_obj;
	pfile_obj_ref_t p_ref;

	//Create buffer
	p_pmo = mm_pmo_create(sizeof(pmsg_open_info_t) + PATH_MAX);

	if(p_pmo == NULL) {
		pm_set_errno(EFAULT);
		return INVALID_FD;
	}

	p_info = mm_pmo_map(NULL, p_pmo, false);

	if(p_info == NULL) {
		mm_pmo_free(p_pmo);
		pm_set_errno(EFAULT);
		return INVALID_FD;
	}

	p_fd = mm_hp_alloc(sizeof(file_desc_t), NULL);

	if(p_fd == NULL) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_pmo_free(p_pmo);
		pm_set_errno(EFAULT);
		return INVALID_FD;
	}

	p_ref = mm_hp_alloc(sizeof(file_obj_ref_t), NULL);

	if(p_ref == NULL) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_hp_free(p_fd, NULL);
		pm_set_errno(EFAULT);
		return INVALID_FD;
	}

	//Create message
	status = msg_create(&p_msg,
	                    sizeof(msg_t));

	if(status != ESUCCESS) {
		mm_hp_free(p_ref, NULL);
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_hp_free(p_fd, NULL);
		pm_set_errno(EFAULT);
		return INVALID_FD;
	}

	p_msg->message = MSG_OPEN;
	p_msg->flags.flags = MFLAG_PMO;
	p_msg->buf.pmo_addr = p_pmo;

	p_info->process = pm_get_crrnt_process();
	p_info->mode = O_RDONLY;
	p_info->flags = 0;

	rtl_strcpy_s(&(p_info->path), PATH_MAX, "initrd");

	//Send message
	status = vfs_send_dev_message(kernel_drv_num,
	                              initrd_ramdisk,
	                              p_msg,
	                              &send_result,
	                              &complete_result);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_hp_free(p_fd, NULL);
		mm_hp_free(p_ref, NULL);
		pm_set_errno(status);
		return INVALID_FD;
	}

	if(send_result != MSTATUS_COMPLETE) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_hp_free(p_fd, NULL);
		mm_hp_free(p_ref, NULL);
		pm_set_errno(EACCES);
		return INVALID_FD;
	}

	if(complete_result != ESUCCESS) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		mm_hp_free(p_fd, NULL);
		mm_hp_free(p_ref, NULL);
		pm_set_errno(complete_result);
		return INVALID_FD;
	}

	//Create file descriptor
	p_fd->file_obj = p_info->file_object;
	p_fd->offset = 0;
	p_file_obj = get_file_obj(p_fd->file_obj);
	p_file_obj->size = p_info->file_size;
	p_fd->flags = 0;
	p_fd->serial_read = p_info->serial_read;
	p_fd->path.p_mount_point = NULL;
	p_fd->path.path = "";

	//Add file descriptor
	p_proc_fd_info = get_proc_fs_info();

	pm_acqr_mutex(&(p_proc_fd_info->lock), TIMEOUT_BLOCK);

	ret = rtl_array_list_get_free_index(&(p_proc_fd_info->file_descs));
	rtl_array_list_set(&(p_proc_fd_info->file_descs), ret, p_fd, NULL);
	ASSERT(OPERATE_SUCCESS);

	pm_rls_mutex(&(p_proc_fd_info->lock));

	mm_pmo_unmap(p_info, p_pmo);
	mm_pmo_free(p_pmo);

	vfs_inc_obj_reference((pkobject_t)p_file_obj);

	//Add process
	p_ref->fd = ret;
	p_ref->process_id = pm_get_crrnt_process();

	pm_acqr_mutex(&(p_file_obj->refered_proc_list_lock), TIMEOUT_BLOCK);
	rtl_list_insert_after(&(p_file_obj->refered_proc_list),
	                      NULL,
	                      p_ref,
	                      NULL);
	pm_rls_mutex(&(p_file_obj->refered_proc_list_lock));

	pm_set_errno(ESUCCESS);

	return ret;
}
