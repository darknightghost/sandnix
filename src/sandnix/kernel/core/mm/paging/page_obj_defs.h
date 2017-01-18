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
#include "../../rtl/rtl_defs.h"
#include "./paging_defs.h"

#define PAGE_OBJ_COPY_ON_WRITE		0x00000001
#define PAGE_OBJ_ALLOC_ON_ACCESS	0x00000002
#define PAGE_OBJ_SWAPPED			0x00000010
#define PAGE_OBJ_SWAPPABLE			0x00000020

//Page object attributes
typedef	struct	_pg_obj_ref_page {
    u32			index;			//Index of page table
    address_t	base_addr;		//Base virtual address
} pg_obj_ref_page_t, *ppg_obj_ref_page_t;

typedef struct _page_obj {
    u32			attr;				//Page object attribute
    size_t		size;				//Size of page
    list_t		copy_on_write_lst;	//List of page object requires to be copied while writting
    //map
    //unmap
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
    struct _page_obj*	(*fork)(struct _page_obj* p_this);

    //Do copy-on-write
    void	(*copy_on_write)(struct _page_obj* p_this);
    union {
        struct {
            address_t	addr;
            size_t		size;
        } phy_mem_info;
        struct {
        } swapped_mem_info;
    };
} page_obj_t, *ppage_obj_t;