/*
    Copyright 2017,王思远 <darknightghost.cn@gmail.com>

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

#pragma once
#include "./file_obj_defs.h"

#define	MODULE_NAME		core_vfs

//Initialize
void	PRIVATE(file_obj_init)();

//Constructor
pfile_obj_t		file_obj(u32 class_id, u32 inode, size_t size);

pfile_obj_t		core_vfs_get_file_obj_by_id(u32 id);

#undef	MODULE_NAME
