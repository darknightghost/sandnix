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

#include "./file_obj.h"
#include "../mm/mm.h"
#include "../pm/pm.h"

#define	MODULE_NAME		core_vfs

static	void			destructor(pfile_obj_t p_this);
static	int				compare(pfile_obj_t p_this, pfile_obj_t p_obj);
static	pkstring_obj_t	to_string(pfile_obj_t p_this);

static	void			die(pfile_obj_t p_this);
static	u32				get_uid(pfile_obj_t p_this);
static	kstatus_t		set_uid(pfile_obj_t p_this, u32 new_uid);
static	u32				get_gid(pfile_obj_t p_this);
static	kstatus_t		set_gid(pfile_obj_t p_this, u32 new_gid);
static	u32				get_mode(pfile_obj_t p_this);
static	kstatus_t		set_mode(pfile_obj_t p_this, u32 new_mode);
static	void			lock(pfile_obj_t p_this);
static	void			unlock(pfile_obj_t p_this);

void	PRIVATE(file_obj_init)();
pfile_obj_t		file_obj(u32 inode, u32 uid, u32 gid, u32 mode, size_t size);
