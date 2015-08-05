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

#include "vfs.h"

void vfs_init()
{

}

//File system
bool vfs_reg_filesystem()
{
	return true;
}

bool vfs_unreg_filesystem()
{
	return true;
}

//Devices
k_status vfs_mount()
{
	return 0;
}

k_status vfs_mount_root()
{
	return 0;
}

k_status vfs_umount()
{
	return 0;
}

//Files
u32 vfs_open()
{
	return 0;
}

bool vfs_inc_fdesc_reference(u32 file_descriptor)
{
	return true;
}

k_status vfs_chmod()
{
	return 0;
}

bool vfs_access()
{
	return true;
}

void vfs_close()
{
	return;
}

size_t vfs_read()
{
	return 0;
}

size_t vfs_write()
{
	return 0;
}

void vfs_sync()
{

}

