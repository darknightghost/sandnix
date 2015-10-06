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

k_status ssddt_mount(va_list p_args)
{
	//Agruments
	char *source;
	char *target;
	char *filesystemtype;
	u32 mountflags;
	char *args;

	//Variables

	//Get args
	source = va_arg(p_args, char*);
	target = va_arg(p_args, char*);
	filesystemtype = va_arg(p_args, char*);
	mountflags = va_arg(p_args, u32);
	args = va_arg(p_args, char*);

	//Check arguments
	if((!check_str_arg(source, PATH_MAX + 1))
	   || (!check_str_arg(target, PATH_MAX + 1))
	   || (!check_str_arg(filesystemtype, NAME_MAX + 1))
	   || (!check_str_arg(args, 4096))) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	return vfs_mount(source, target, filesystemtype, mountflags, args);
}

k_status ssddt_umount(va_list p_args)
{
	//Agruments
	char *target;

	//Variables

	//Get args
	target = va_arg(p_args, char*);

	//Check arguments
	if(!check_str_arg(target, PATH_MAX + 1)) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	return vfs_umount(target);
}
