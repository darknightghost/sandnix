/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

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

#include "../../../../../common/common.h"
#include "../../rtl/container/list/list_defs.h"
#include "../../rtl/obj/obj_defs.h"
#include "./paging_defs.h"

#define PAGE_OBJ_ALLOCATED			0x00000001
#define	PAGE_OBJ_DMA				0x00000002
#define PAGE_OBJ_COPY_ON_WRITE		0x00000004
#define PAGE_OBJ_CAHCEABLE			0x00000008
#define PAGE_OBJ_SWAPPED			0x00000010
#define PAGE_OBJ_SWAPPABLE			0x00000020

#define	PAGE_OBJ_ATTR_MASK			PAGE_OBJ_SWAPPABLE

typedef struct _page_obj {
    obj_t		obj;
    u32			attr;				//Page object attribute
    size_t		size;				//Size of page
    struct {
        struct _page_obj* p_prev;
        struct _page_obj* p_next;
    } copy_on_write_ref;			//Copy on write reference info
    //Swap page
    void	(*swap)(struct _page_obj* p_this);

    //Unswap page
    void	(*unswap)(struct _page_obj* p_this);

    //Set attribute
    void	(*set_attr)(struct _page_obj* p_this, u32 attr);

    //Get attribute
    u32(*get_attr)(struct _page_obj* p_this);

    //Get page size
    size_t	(*get_size)(struct _page_obj* p_this);

    //Fork page object
    struct _page_obj *	(*fork)(struct _page_obj* p_this);

    //Do copy-on-write
    void	(*copy_on_write)(struct _page_obj* p_this, void* virt_addr,
                             u32 attr);
    //Map
    void	(*map)(struct _page_obj* p_this, void* virt_addr, u32 attr);

    //Unmap
    void	(*unmap)(struct _page_obj* p_this, void* virt_addr);

    //Allocate physical memory
    void	(*alloc)(struct _page_obj* p_this);

    //Check if the memory has been allocated
    bool	(*is_alloced)(struct _page_obj* p_this);
    union {
        struct {
            address_t	addr;
        } phy_mem_info;
        struct {
        } swapped_mem_info;
    } mem_info;
} page_obj_t, *ppage_obj_t;
