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
#include "../rtl/rtl.h"
#include "../exception/exception.h"
#include "../../hal/mmu/mmu.h"

#define	MODULE_NAME		core_vfs

//Heap
static	pheap_t			file_obj_heap = NULL;

//File object table
static	array_t			file_obj_table;
static	mutex_t			file_obj_table_lock;

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
static	void			lock_obj(pfile_obj_t p_this);
static	void			unlock_obj(pfile_obj_t p_this);

void PRIVATE(file_obj_init)()
{
    //Create heap
    file_obj_heap = core_mm_heap_create(HEAP_MULITHREAD, SANDNIX_KERNEL_PAGE_SIZE);

    if(file_obj_heap == NULL) {
        penomem_except_t p_except = enomem_except();
        RAISE(p_except, "Failed to create heap for file objects.");
        return;
    }

    //Initialze file object table
    core_rtl_array_init(&file_obj_table, MAX_FILEOBJ_NUM, file_obj_heap);
    core_pm_mutex_init(&file_obj_table_lock);

    return;
}

pfile_obj_t file_obj(u32 class_id, u32 inode, u32 uid, u32 gid, u32 mode, size_t size)
{
    //Check size
    if(size < sizeof(file_obj_t)) {
        return NULL;
    }

    //Allocate id
    core_pm_mutex_acquire(&file_obj_table_lock, -1);
    u32 new_id;

    if(core_rtl_array_get_free_index(
           &file_obj_table,
           &new_id) == false) {
        core_pm_mutex_release(&file_obj_table_lock);
        return NULL;
    }

    //Create object
    pfile_obj_t p_ret = (pfile_obj_t)obj(
                            class_id,
                            (destructor_t)destructor,
                            (compare_obj_t)compare,
                            (to_string_t)to_string,
                            file_obj_heap,
                            size);

    if(p_ret == NULL) {
        core_pm_mutex_release(&file_obj_table_lock);
        return NULL;
    }

    //Attributes
    p_ret->file_obj_id = new_id;
    core_rtl_array_set(&file_obj_table, new_id, p_ret);
    core_pm_mutex_release(&file_obj_table_lock);
    core_pm_mutex_init(&(p_ret->lock));
    p_ret->alive = true;
    p_ret->inode = inode;
    p_ret->uid = uid;
    p_ret->gid = gid;
    p_ret->mode = mode;

    //Methods
    p_ret->die = die;
    p_ret->get_uid = get_uid;
    p_ret->set_uid = set_uid;
    p_ret->get_gid = get_gid;
    p_ret->set_gid = set_gid;
    p_ret->get_mode = get_mode;
    p_ret->set_mode = set_mode;
    p_ret->lock_obj = lock_obj;
    p_ret->unlock_obj = unlock_obj;

    return p_ret;
}

void  destructor(pfile_obj_t p_this);
int compare(pfile_obj_t p_this, pfile_obj_t p_obj);
pkstring_obj_t to_string(pfile_obj_t p_this);

void die(pfile_obj_t p_this);
u32 get_uid(pfile_obj_t p_this);
kstatus_t set_uid(pfile_obj_t p_this, u32 new_uid);
u32 get_gid(pfile_obj_t p_this);
kstatus_t set_gid(pfile_obj_t p_this, u32 new_gid);
u32 get_mode(pfile_obj_t p_this);
kstatus_t set_mode(pfile_obj_t p_this, u32 new_mode);
void lock_obj(pfile_obj_t p_this);
void unlock_obj(pfile_obj_t p_this);
