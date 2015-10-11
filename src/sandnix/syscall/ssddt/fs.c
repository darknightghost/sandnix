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

u32 ssddt_open(va_list p_args)
{
	//Agruments
	char* pathname;
	u32 flags;
	u32 mode;

	//Variables

	//Get args
	pathname = va_arg(p_args, char*);
	flags = va_arg(p_args, u32);
	mode = va_arg(p_args, u32);

	//Check arguments
	if(!check_str_arg(pathname, PATH_MAX + 1)) {
		pm_set_errno(EINVAL);
		return 0;
	}

	return vfs_open(pathname, flags, mode);
}

k_status ssddt_fchmod(va_list p_args)
{
	//Agruments
	u32 fd;
	u32 mode;

	//Variables

	//Get args
	fd = va_arg(p_args, u32);
	mode = va_arg(p_args, u32);

	return vfs_chmod(fd, mode);
}

k_status ssddt_access(va_list p_args)
{
	//Agruments
	char *pathname;
	u32 mode;

	//Variables

	//Get args
	pathname = va_arg(p_args, char*);
	mode = va_arg(p_args, u32);

	//Check arguments
	if(!check_str_arg(pathname, PATH_MAX + 1)) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	return vfs_access(pathname, mode);
}

void ssddt_close(va_list p_args)
{
	//Agruments
	u32 fd;

	//Variables

	//Get args
	fd = va_arg(p_args, u32);

	vfs_close(fd);
	return;
}

size_t ssddt_read(va_list p_args)
{
	//Agruments
	u32 fd;
	void *buf;
	size_t count;

	//Variables
	size_t ret;
	k_status status;
	ppmo_t p_pmo;
	pmsg_read_info_t p_info;
	pmsg_read_data_t p_data;

	//Get args
	fd = va_arg(p_args, u32);
	buf = va_arg(p_args, void*);
	count = va_arg(p_args, size_t);

	//Check arguments
	if(!mm_virt_test(buf, count, PG_STAT_USER | PG_STAT_WRITEABLE, true)) {
		pm_set_errno(EINVAL);
		return 0;
	}

	//Create&map pmo
	p_pmo = mm_pmo_create(sizeof(msg_read_info_t) + count);

	if(p_pmo == NULL) {
		return 0;
	}

	p_info = mm_pmo_map(NULL, p_pmo, false);

	if(p_info == NULL) {
		status = pm_get_errno();
		mm_pmo_free(p_pmo);
		pm_set_errno(status);
		return 0;
	}

	//Read file
	p_data = (pmsg_read_data_t)p_info;
	p_info->len = count;
	status = vfs_read(fd, p_pmo);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		pm_set_errno(status);
		return 0;
	}

	ret = p_data->len;
	rtl_memcpy(buf, &(p_data->data), ret);
	mm_pmo_unmap(p_info, p_pmo);
	mm_pmo_free(p_pmo);
	pm_set_errno(ESUCCESS);

	return ret;
}

size_t ssddt_write(va_list p_args)
{
	//Agruments
	u32 fd;
	void *buf;
	size_t count;

	//Variables
	size_t ret;
	k_status status;
	ppmo_t p_pmo;
	pmsg_write_info_t p_info;

	//Get args
	fd = va_arg(p_args, u32);
	buf = va_arg(p_args, void*);
	count = va_arg(p_args, size_t);

	//Check arguments
	if(!mm_virt_test(buf, count, PG_STAT_USER | PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return 0;
	}

	//Create&map pmo
	p_pmo = mm_pmo_create(sizeof(msg_write_info_t) + count);

	if(p_pmo == NULL) {
		return 0;
	}

	p_info = mm_pmo_map(NULL, p_pmo, false);

	if(p_info == NULL) {
		status = pm_get_errno();
		mm_pmo_free(p_pmo);
		pm_set_errno(status);
		return 0;
	}

	//Write file
	p_info->len = count;
	rtl_memcpy(&(p_info->data), buf, count);
	ret = vfs_write(fd, p_pmo);
	status = pm_get_errno();
	mm_pmo_unmap(p_info, p_pmo);
	mm_pmo_free(p_pmo);

	pm_set_errno(status);
	return ret;
}

k_status ssddt_seek(va_list p_args)
{
	//Agruments
	u32 fd;
	s64 offset;
	u32 whence;
	u64* p_after_pos;

	//Variables

	//Get args
	fd = va_arg(p_args, u32);
	offset = va_arg(p_args, u64);
	whence = va_arg(p_args, u32);
	p_after_pos = va_arg(p_args, u64*);

	//Check arguments
	if(!mm_virt_test(p_after_pos, sizeof(u64)
	                 , PG_STAT_USER | PG_STAT_WRITEABLE, true)) {
		pm_set_errno(EINVAL);
		return 0;
	}

	*p_after_pos = vfs_seek(fd, whence, offset);

	return pm_get_errno();
}

k_status ssddt_stat(va_list p_args)
{
	//Agruments
	char *path;
	pfile_stat_t buf;

	//Variables
	k_status status;
	ppmo_t p_pmo;
	pmsg_stat_info_t p_info;
	pmsg_stat_data_t p_data;
	size_t path_len;

	//Get args
	path = va_arg(p_args, char*);
	buf = va_arg(p_args, pfile_stat_t);

	//Check arguments
	if((!mm_virt_test(buf, sizeof(file_stat_t)
	                  , PG_STAT_USER | PG_STAT_WRITEABLE, true)
	    || (!check_str_arg(path, PATH_MAX + 1)))) {
		pm_set_errno(EINVAL);
		return 0;
	}

	//Create&map pmo
	path_len = rtl_strlen(path) + 1;
	p_pmo = mm_pmo_create(sizeof(msg_stat_info_t) > path_len
	                      ? sizeof(msg_stat_info_t)
	                      : path_len);

	if(p_pmo == NULL) {
		return 0;
	}

	p_info = mm_pmo_map(NULL, p_pmo, false);

	if(p_info == NULL) {
		status = pm_get_errno();
		mm_pmo_free(p_pmo);
		pm_set_errno(status);
		return 0;
	}

	//Get file stat
	p_data = (pmsg_stat_data_t)p_info;
	status = vfs_stat(path, p_pmo);

	if(status == ESUCCESS) {
		rtl_memcpy(buf, &(p_data->stat), sizeof(file_stat_t));
	}

	mm_pmo_unmap(p_info, p_pmo);
	mm_pmo_free(p_pmo);
	pm_set_errno(status);

	return status;
}

k_status ssddt_unlink(va_list p_args)
{
	//Agruments
	char *pathname;

	//Variables

	//Get args
	pathname = va_arg(p_args, char*);

	//Check arguments
	if(!check_str_arg(pathname, PATH_MAX + 1)) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	return vfs_unlink(pathname);
}

k_status ssddt_mkdir(va_list p_args)
{
	//Agruments
	char *pathname;
	u32 mode;

	//Variables

	//Get args
	pathname = va_arg(p_args, char*);
	mode = va_arg(p_args, u32);

	//Check arguments
	if(!check_str_arg(pathname, PATH_MAX + 1)) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	return vfs_mkdir(pathname, mode);
}

size_t ssddt_readdir(va_list p_args)
{
	//Agruments
	u32 fd;
	pdirent_t buf;
	size_t count;

	//Variables
	size_t ret;
	k_status status;
	ppmo_t p_pmo;
	pmsg_readdir_info_t p_info;
	pmsg_readdir_data_t p_data;

	//Get args
	fd = va_arg(p_args, u32);
	buf = va_arg(p_args, pdirent_t);
	count = va_arg(p_args, size_t);

	//Check arguments
	if(!mm_virt_test(buf, sizeof(count)
	                 , PG_STAT_USER | PG_STAT_WRITEABLE, true)) {
		pm_set_errno(EINVAL);
		return 0;
	}

	//Create&map pmo
	p_pmo = mm_pmo_create(sizeof(msg_readdir_info_t) + count);

	if(p_pmo == NULL) {
		return 0;
	}

	p_info = mm_pmo_map(NULL, p_pmo, false);

	if(p_info == NULL) {
		status = pm_get_errno();
		mm_pmo_free(p_pmo);
		pm_set_errno(status);
		return 0;
	}

	//Read file
	p_data = (pmsg_readdir_data_t)p_info;
	p_info->count = count;
	status = vfs_readdir(fd, p_pmo);

	if(status != ESUCCESS) {
		mm_pmo_unmap(p_info, p_pmo);
		mm_pmo_free(p_pmo);
		pm_set_errno(status);
		return 0;
	}

	ret = p_data->count;
	rtl_memcpy(buf, &(p_data->data), ret);
	mm_pmo_unmap(p_info, p_pmo);
	mm_pmo_free(p_pmo);
	pm_set_errno(ESUCCESS);

	return ret;
}
