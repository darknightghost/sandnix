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


#include "../../../../../common/common.h"

#include "./page_obj.h"
#include "../../rtl/rtl.h"
#include "../heap/heap.h"

#include "../../../hal/mmu/mmu.h"
#include "../../../hal/exception/exception.h"

static	pheap_t			page_obj_heap = NULL;
static	u8				page_obj_heap_buf[SANDNIX_KERNEL_PAGE_SIZE];

//Object
static	void			destructer(ppage_obj_t p_this);
static	int				compare(ppage_obj_t p_this, ppage_obj_t p_str);
static	pkstring_obj_t	to_string(ppage_obj_t p_this);

//Metods
//Swap
static	void		swap(ppage_obj_t p_this);
//Unswap page
static	void		unswap(ppage_obj_t p_this);

//Set attribute
static	void		set_attr(ppage_obj_t p_this, u32 attr);

//Get attribute
static	u32			get_attr(ppage_obj_t p_this);

//Get page size
static	size_t		get_size(ppage_obj_t p_this);

//Fork page object
static	ppage_obj_t	fork(ppage_obj_t p_this);

//Do copy-on-write
static	void		copy_on_write(ppage_obj_t p_this);

ppage_obj_t page_obj(size_t page_size, u32 attr)
{
    if(page_obj_heap == NULL) {
        //Initialize heap
        page_obj_heap = core_mm_heap_create_on_buf(
                            HEAP_MULITHREAD | HEAP_PREALLOC,
                            SANDNIX_KERNEL_PAGE_SIZE,
                            page_obj_heap_buf,
                            sizeof(page_obj_heap_buf));

        if(page_obj_heap == NULL) {
            PANIC(ENOMEM,
                  "Failed to create heap for page objects.");
        }
    }

    ppage_obj_t p_ret = (ppage_obj_t)obj(CLASS_PAGE_OBJECT,
                                         (destructor_t)destructer,
                                         (compare_obj_t)compare,
                                         (to_string_t)to_string,
                                         page_obj_heap,
                                         sizeof(page_obj_t));

    if(p_ret == NULL) {
        return NULL;
    }

    //Set methods
    p_ret->swap = swap;
    p_ret->unswap = unswap;
    p_ret->set_attr = set_attr;
    p_ret->get_attr = get_attr;
    p_ret->fork = fork;
    p_ret->copy_on_write = copy_on_write;


}

ppage_obj_t page_obj_on_phymem(address_t phy_base, size_t size);
