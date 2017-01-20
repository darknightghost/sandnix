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
#include "../../rtl/container/map/map_defs.h"
#include "./page_obj_defs.h"

//Page attributes
#define	PAGE_OPTION_COMMIT		0x00000001
#define	PAGE_OPTION_WRITABLE	0x00000002
#define	PAGR_OPTION_EXECUTABLE	0x00000004
#define	PAGE_OPTION_SWAPPABLE	0x00000008
#define	PAGE_OPTION_DMA			0x00000010
#define	PAGE_OPTION_KERNEL		0x80000000

#define	PAGE_ALLOCATED			0x00000001

#define	PAGE_AVAIL				0x00000002

#define PAGE_WRITABLE			0x00000004
#define PAGE_EXECUTABLE			0x00000008

#define PAGE_SWAPPABLE			0x00000010

#define	PAGE_DMA				0x00000020

#define PAGE_BLOCK_ALLOCATED	0x00000001
#define PAGE_BLOCK_COMMITED		0x00000002
#define	PAGE_BLOCK_WRITEABLE	0x00000004
#define	PAGE_BLOCK_EXECUTABLE	0x00000008

//Page block
typedef struct	_page_obj		page_obj_t, *ppage_obj_t;
typedef struct	_page_block {
    struct _page_block*	p_prev;		//Prev block
    struct _page_block*	p_next;		//Next block
    address_t			begin;		//Base address
    size_t				size;		//Size
    u32					status;		//Page status
    ppage_obj_t			p_pg_obj;	//Page object
} page_block_t, *ppage_block_t;

//Page table of process
typedef	struct	_proc_pg_tbl {
    u32		id;						//Process id(Page table id)
    map_t	used_map;				//Used page blocks
    map_t	free_addr_map;			//Free page blocks, sorted by address
    map_t	free_size_map;			//Free page blocks, sorted by size
} proc_pg_tbl_t, *proc_pg_tbl;
