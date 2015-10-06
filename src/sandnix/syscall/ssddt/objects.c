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

u32 ssddt_create_file_obj()
{
	return create_file_obj();
}

u32 ssddt_create_drv_obj(va_list p_args)
{
	//Agruments
	char* drv_name;

	//Variables
	pdriver_obj_t p_driver;

	//Get args
	drv_name = va_arg(p_args, char*);

	//Check arguments
	if(!check_str_arg(drv_name, NAME_MAX + 1)) {
		pm_set_errno(EINVAL);
		return 0;
	}

	//Create &regist driver object
	p_driver = vfs_create_drv_object(drv_name);

	if(p_driver == NULL) {
		return 0;
	}

	p_driver->process_id = pm_get_crrnt_process();
	vfs_reg_driver(p_driver);
	return p_driver->driver_id;
}

u32 ssddt_create_dev(va_list p_args)
{
	//Agruments
	char* name;
	u32 major;
	u32 gid;
	u32 block_size;
	u32 parent;

	//Variables
	pdevice_obj_t p_dev;

	//Get args
	name = va_arg(p_args, char*);
	major = va_arg(p_args, u32);
	gid = va_arg(p_args, gid);
	block_size = va_arg(p_args, u32);
	parent = va_arg(p_args, u32);

	//Check arguments
	if(!check_str_arg(name, NAME_MAX)) {
		pm_set_errno(EINVAL);
		return 0;
	}

	//Create&regist device object
	p_dev = vfs_create_dev_object(name);

	if(p_dev == NULL) {
		return 0;
	}

	p_dev->device_number = MK_DEV(major, 0);
	p_dev->gid = gid;
	p_dev->block_size = block_size;
	p_dev->parent_dev = parent;

	vfs_add_device(p_dev, vfs_get_crrnt_driver_id());

	return p_dev->device_number;
}

void ssddt_remove_dev(va_list p_args)
{
	//Agruments
	u32 dev_num;

	//Variables

	//Get args
	dev_num = va_arg(p_args, u32);

	vfs_remove_device(dev_num);
	return;
}

k_status ssddt_set_dev_filename(va_list p_args)
{
	//Agruments
	u32 dev_num;
	char* file_name;

	//Variables

	//Get args
	dev_num = va_arg(p_args, u32);
	file_name = va_arg(p_args, char*);

	//Check arguments
	if(!check_str_arg(file_name, NAME_MAX + 1)) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	return vfs_set_dev_filename(dev_num, file_name);
}

u32 ssddt_get_major(va_list p_args)
{
	//Agruments
	char* name;
	u32 type;

	//Variables

	//Get args
	name = va_arg(p_args, char*);
	type = va_arg(p_args, u32);

	//Check arguments
	if(!check_str_arg(name, NAME_MAX + 1)) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	return vfs_get_dev_major_by_name(name, type);
}

k_status ssddt_sync(va_list p_args)
{
	//Agruments
	u32 dev_num;

	//Variables

	//Get args
	dev_num = va_arg(p_args, dev_num);

	vfs_sync(dev_num);

	return pm_get_errno();
}
