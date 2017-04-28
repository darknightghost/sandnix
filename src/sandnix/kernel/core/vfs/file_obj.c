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
static	kstatus_t		set_uid(pfile_obj_t p_this, u32 new_uid, u32 process_id);
static	u32				get_gid(pfile_obj_t p_this);
static	kstatus_t		set_gid(pfile_obj_t p_this, u32 new_gid, u32 process_id);
static	u32				get_mode(pfile_obj_t p_this);
static	kstatus_t		set_mode(pfile_obj_t p_this, u32 new_mode, u32 process_id);
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

pfile_obj_t core_vfs_get_file_obj_by_id(u32 id)
{
    core_pm_mutex_acquire(&file_obj_table_lock, -1);
    pfile_obj_t p_ret = core_rtl_array_get(&file_obj_table, id);

    if(p_ret != NULL) {
        INC_REF(p_ret);
    }

    core_pm_mutex_release(&file_obj_table_lock);

    return p_ret;
}

void destructor(pfile_obj_t p_this)
{
    //Release id
    core_pm_mutex_acquire(&file_obj_table_lock, -1);
    core_rtl_array_set(&file_obj_table, p_this->file_obj_id, NULL);
    core_pm_mutex_release(&file_obj_table_lock);

    //Free memory
    core_mm_heap_free(p_this, p_this->obj.heap);
    return;
}

int compare(pfile_obj_t p_this, pfile_obj_t p_obj)
{
    if((address_t)p_this > (address_t)p_obj) {
        return 1;

    } else if((address_t)p_this == (address_t)p_obj) {
        return 0;

    } else {
        return -1;
    }
}

pkstring_obj_t to_string(pfile_obj_t p_this)
{
    return kstring("File object.", p_this->obj.heap);
}

void die(pfile_obj_t p_this)
{
    p_this->lock_obj(p_this);
    p_this->alive = false;
    p_this->unlock_obj(p_this);

    return;
}

u32 get_uid(pfile_obj_t p_this)
{
    p_this->lock_obj(p_this);

    if(!p_this->alive) {
        p_this->unlock_obj(p_this);
        core_exception_set_errno(EIO);
        return 0;
    }

    u32 ret = p_this->uid;
    p_this->unlock_obj(p_this);

    core_exception_set_errno(ESUCCESS);
    return ret;
}

kstatus_t set_uid(pfile_obj_t p_this, u32 new_uid, u32 process_id)
{
    //Check privilege
    if(process_id != 0) {
        core_exception_set_errno(EPERM);
        return EPERM;
    }

    p_this->lock_obj(p_this);

    //Check if the file object is alive
    if(!p_this->alive) {
        p_this->unlock_obj(p_this);
        core_exception_set_errno(EIO);
        return EIO;
    }

    p_this->uid = new_uid;

    p_this->unlock_obj(p_this);
    core_exception_set_errno(ESUCCESS);

    return ESUCCESS;
}

u32 get_gid(pfile_obj_t p_this)
{
    p_this->lock_obj(p_this);

    if(!p_this->alive) {
        p_this->unlock_obj(p_this);
        core_exception_set_errno(EIO);
        return 0;
    }

    u32 ret = p_this->gid;
    p_this->unlock_obj(p_this);

    core_exception_set_errno(ESUCCESS);
    return ret;
}

kstatus_t set_gid(pfile_obj_t p_this, u32 new_gid, u32 process_id)
{
    //Check privilege
    if(process_id != 0) {
        if(core_pm_get_euid(process_id) == p_this->uid) {
            if(core_pm_get_gid(process_id) != new_gid) {
                core_exception_set_errno(EPERM);
                return EPERM;

            } else {
                size_t group_buf_size = core_pm_get_groups(
                                            process_id,
                                            NULL,
                                            0);

                if(group_buf_size == 0) {
                    core_exception_set_errno(EPERM);
                    return EPERM;
                }

                u32* p_group_buf = core_mm_heap_alloc(group_buf_size,
                                                      file_obj_heap);
                bool found = false;

                for(u32* p = p_group_buf;
                    p - p_group_buf < group_buf_size / sizeof(u32);
                    p++) {
                    if(*p == new_gid) {
                        found = true;
                    }
                }

                core_mm_heap_free(p_group_buf, file_obj_heap);

                if(!found) {
                    core_exception_set_errno(EPERM);
                    return EPERM;
                }

            }

        } else {
            core_exception_set_errno(EPERM);
            return EPERM;
        }
    }

    p_this->lock_obj(p_this);

    //Check if the file object is alive
    if(!p_this->alive) {
        p_this->unlock_obj(p_this);
        core_exception_set_errno(EIO);
        return EIO;
    }

    p_this->gid = new_gid;

    p_this->unlock_obj(p_this);
    core_exception_set_errno(ESUCCESS);

    return ESUCCESS;
}

u32 get_mode(pfile_obj_t p_this)
{
    p_this->lock_obj(p_this);

    if(!p_this->alive) {
        p_this->unlock_obj(p_this);
        core_exception_set_errno(EIO);
        return 0;
    }

    u32 ret = p_this->mode;
    p_this->unlock_obj(p_this);

    core_exception_set_errno(ESUCCESS);
    return ret;
}

kstatus_t set_mode(pfile_obj_t p_this, u32 new_mode, u32 process_id)
{
    //Check privilege
    if(process_id != 0) {
        core_exception_set_errno(EPERM);
        return EPERM;
    }

    p_this->lock_obj(p_this);

    //Check if the file object is alive
    if(!p_this->alive) {
        p_this->unlock_obj(p_this);
        core_exception_set_errno(EIO);
        return EIO;
    }

    p_this->mode = new_mode;

    p_this->unlock_obj(p_this);
    core_exception_set_errno(ESUCCESS);

    return ESUCCESS;
}

void lock_obj(pfile_obj_t p_this)
{
    core_pm_mutex_acquire(&(p_this->lock), -1);
    return;
}

void unlock_obj(pfile_obj_t p_this)
{
    core_pm_mutex_release(&(p_this->lock));
    return;
}

