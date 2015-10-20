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

#ifndef	OM_H_INCLUDE
#define	OM_H_INCLUDE

#include "../../pm/pm.h"
#include "../../rtl/rtl.h"
#include "../fs/fs.h"

typedef	struct	_kobject	kobject_t, *pkobject_t;

typedef	void	(*obj_destroyer)(pkobject_t);

struct	_kobject {
	char*				name;
	u32					class_num;
	size_t				size;
	obj_destroyer		destroy_callback;
	u32					ref_count;
	mutex_t				ref_count_lock;
};

#define	OBJ_MAJOR_CLASS(p_object)	(((p_object)->class_num) & 0xFFFF0000)
#define	OBJ_MINOR_CLASS(p_object)	(((p_object)->class_num) & 0x0000FFFF)

//Major class
#define	OBJ_MJ_DRIVER				0x00010000
#define	OBJ_MJ_FILE					0x00020000

//Minor class
//Drivers
#define	OBJ_MN_FS					0x00000001

//Files
#define	OBJ_MN_NORMAL				0x00000001
#define	OBJ_MN_DEVICE				0x00000002

typedef	struct	_driver_obj {
	kobject_t		obj;
	u32				process_id;
	u32				driver_id;
	bool			destroy_flag;
	u32				msg_queue;
	list_t			file_list;
	mutex_t			file_list_lock;
} driver_obj_t, *pdriver_obj_t;

typedef	struct	_file_obj {
	kobject_t		obj;
	pdriver_obj_t	p_driver;
	u32				file_id;
	u64				size;
	list_t			refered_proc_list;
	mutex_t			refered_proc_list_lock;
} file_obj_t, *pfile_obj_t;

typedef	struct	_device_obj {
	file_obj_t		file_obj;
	u32				device_number;
	bool			has_parent;
	u32				parent_dev;
	u32				gid;
	size_t			block_size;
	void*			p_additional;
	obj_destroyer	additional_destroyer;
	list_t			child_list;
	mutex_t			child_list_lock;
} device_obj_t, *pdevice_obj_t;

#define	DEV_TYPE_CHAR		0x01
#define	DEV_TYPE_BLOCK		0x02

#define	DEV_MJ_NUM_MAX		512
#define	DEV_MN_NUM_MAX		512

#define	DEV_NUM_MJ(dev_num)	((0xFFFF0000 & (dev_num)) >> 16)
#define	DEV_NUM_MN(dev_num)	(0x0000FFFF & (dev_num))
#define	MK_DEV(mj,mn)		(((mj) << 16) | (0x0000FFFF & (mn)))

#define	INVALID_DEV			0xFFFFFFFF
#define	INVALID_DRV			0xFFFFFFFF

typedef	struct	{
	file_obj_t		file_obj;
	u32				mj_num;
	array_list_t	devices;
	u32				device_type;
	mutex_t			lock;
	u32				devices_count;
} dev_mj_info_t, *pdev_mj_info_t;

typedef	struct _dirent	dirent_t, *pdirent_t;

extern	u32		devfs_driver;

void			om_init();

pdriver_obj_t	get_driver(u32 driver_id);
pdevice_obj_t	get_dev_by_name(char* name);
pdev_mj_info_t	get_mj_by_name(char* name);
size_t			get_devfs_root(pdirent_t buf, size_t offset, size_t count);
size_t			get_devfs_dir(pdev_mj_info_t p_dir, pdirent_t buf,
                              size_t offset, size_t count);
pdevice_obj_t	get_dev(u32 dev_num);
void			devfs_init();

#endif	//!	OM_H_INCLUDE
