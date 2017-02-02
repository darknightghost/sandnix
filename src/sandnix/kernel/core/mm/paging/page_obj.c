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
#include "./paging.h"
#include "../../rtl/rtl.h"
#include "../heap/heap.h"

#include "../../pm/pm.h"

#include "../../../hal/mmu/mmu.h"
#include "../../../hal/exception/exception.h"

static	pheap_t			page_obj_heap = NULL;
static	u8				page_obj_heap_buf[SANDNIX_KERNEL_PAGE_SIZE];
static	__attribute__((aligned(SANDNIX_KERNEL_PAGE_SIZE))) u8	page_copy_buf[
     SANDNIX_KERNEL_PAGE_SIZE];

//Object
static	void			destructer(ppage_obj_t p_this);
static	int				compare(ppage_obj_t p_this, ppage_obj_t p_1);
static	pkstring_obj_t	to_string(ppage_obj_t p_this);

//Methods
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
static	void		copy_on_write(ppage_obj_t p_this, void* virt_addr,
                                  u32 attr);

//Map
static	void		map(ppage_obj_t p_this, void* virt_addr, u32 attr);

//Unmap
static	void		unmap(ppage_obj_t p_this, void* virt_addr);

//Allocate physical memory
static	void		alloc(ppage_obj_t p_this);

//Check if the page object has been allocated
static	bool		is_alloced(ppage_obj_t p_this);

//Private methods
static	void		copy_on_write_unref(ppage_obj_t p_this);

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

    //Set attributes
    p_ret->attr = attr & (PAGE_OBJ_SWAPPABLE | PAGE_OBJ_DMA
                          | PAGE_OBJ_CAHCEABLE);
    p_ret->size = page_size;
    p_ret->copy_on_write_ref.p_prev = NULL;
    p_ret->copy_on_write_ref.p_next = NULL;

    //Set methods
    p_ret->swap = swap;
    p_ret->unswap = unswap;
    p_ret->set_attr = set_attr;
    p_ret->get_attr = get_attr;
    p_ret->get_size = get_size;
    p_ret->fork = fork;
    p_ret->copy_on_write = copy_on_write;
    p_ret->map = map;
    p_ret->unmap = unmap;
    p_ret->alloc = alloc;
    p_ret->is_alloced = is_alloced;

    return p_ret;
}

ppage_obj_t page_obj_on_phymem(address_t phy_base, size_t size, bool cacheable)
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

    //Set attributes
    p_ret->attr = PAGE_OBJ_ALLOCATED;

    if(cacheable) {
        p_ret->attr |= PAGE_OBJ_CAHCEABLE;
    }

    p_ret->size = size;
    p_ret->copy_on_write_ref.p_prev = NULL;
    p_ret->copy_on_write_ref.p_next = NULL;
    p_ret->mem_info.phy_mem_info.addr = phy_base;

    //Set methods
    p_ret->swap = swap;
    p_ret->unswap = unswap;
    p_ret->set_attr = set_attr;
    p_ret->get_attr = get_attr;
    p_ret->get_size = get_size;
    p_ret->fork = fork;
    p_ret->copy_on_write = copy_on_write;
    p_ret->map = map;
    p_ret->unmap = unmap;
    p_ret->alloc = alloc;
    p_ret->is_alloced = is_alloced;

    return p_ret;
}

void destructer(ppage_obj_t p_this)
{
    if(p_this->attr & PAGE_OBJ_ALLOCATED) {
        if(p_this->attr & PAGE_OBJ_COPY_ON_WRITE) {
            if(p_this->attr & PAGE_OBJ_SWAPPED) {
                //TODO:Decrease swapped memory reference
                NOT_SUPPORT;

            } else {
                //Unreference memory
                copy_on_write_unref(p_this);
            }

        } else {
            if(p_this->attr & PAGE_OBJ_SWAPPED) {
                //TODO:Free swapped memory
                NOT_SUPPORT;

            } else {
                //Free memory
                hal_mmu_phymem_free((void*)(p_this->mem_info.phy_mem_info.addr));
            }
        }
    }

    core_mm_heap_free(p_this, p_this->obj.heap);

    return;
}

int compare(ppage_obj_t p_this, ppage_obj_t p_1)
{
    if(p_this->size > p_1->size) {
        return 1;

    } else if(p_this->size == p_1->size) {
        return 0;

    } else {
        return -1;
    }
}

pkstring_obj_t to_string(ppage_obj_t p_this)
{
    if(p_this->attr & PAGE_OBJ_ALLOCATED) {
        char str[64] = {0};

        core_rtl_strncat(str, "\"Allocated\" ", sizeof(str));

        if(p_this->attr & PAGE_OBJ_COPY_ON_WRITE) {
            core_rtl_strncat(str, "\"Copy on write\" ", sizeof(str));
        }

        if(p_this->attr & PAGE_OBJ_SWAPPED) {
            core_rtl_strncat(str, "\"Swapped\" ", sizeof(str));

        } else if(p_this->attr & PAGE_OBJ_SWAPPABLE) {
            core_rtl_strncat(str, "\"Swappable\" ", sizeof(str));
        }

        if(p_this->attr & PAGE_OBJ_SWAPPED) {
            //TODO:
            NOT_SUPPORT;
            return NULL;

        } else {
            return kstring_fmt("Page object at %p\n"
                               "Size = %p\n"
                               "Status : %s\n"
                               "Physical memory address : %p",
                               p_this->obj.heap,
                               p_this,
                               p_this->size,
                               str,
                               p_this->mem_info.phy_mem_info.addr);
        }

    } else {
        //Not allocated
        return kstring_fmt("Page object at %p\n"
                           "Size = %p\n"
                           "Status : \"Not allocated.\"",
                           p_this->obj.heap,
                           p_this,
                           p_this->size);
    }
}

void swap(ppage_obj_t p_this)
{
    NOT_SUPPORT;
    UNREFERRED_PARAMETER(p_this);
}

void unswap(ppage_obj_t p_this)
{
    NOT_SUPPORT;
    UNREFERRED_PARAMETER(p_this);
}

void set_attr(ppage_obj_t p_this, u32 attr)
{
    p_this->attr = (p_this->attr
                    & (~PAGE_OBJ_ATTR_MASK))
                   | (attr & PAGE_OBJ_ATTR_MASK);

    return;
}

u32 get_attr(ppage_obj_t p_this)
{
    return p_this->attr;
}

size_t get_size(ppage_obj_t p_this)
{
    return p_this->size;
}

ppage_obj_t fork(ppage_obj_t p_this)
{
    ppage_obj_t p_ret = (ppage_obj_t)obj(CLASS_PAGE_OBJECT,
                                         (destructor_t)destructer,
                                         (compare_obj_t)compare,
                                         (to_string_t)to_string,
                                         page_obj_heap,
                                         sizeof(page_obj_t));

    if(p_ret == NULL) {
        return NULL;
    }


    //Set attributes
    if(p_ret->attr & PAGE_OBJ_ALLOCATED) {
        //Allocated
        p_ret->attr = p_this->attr | PAGE_OBJ_COPY_ON_WRITE;
        p_this->attr = p_ret->attr;
        p_ret->size = p_this->size;
        core_rtl_memcpy(&(p_ret->mem_info), &(p_this->mem_info),
                        sizeof(p_this->mem_info));

        //Copy on write reference
        p_ret->copy_on_write_ref.p_next = p_this->copy_on_write_ref.p_next;
        p_this->copy_on_write_ref.p_next->copy_on_write_ref.p_prev = p_ret;
        p_this->copy_on_write_ref.p_next = p_ret;
        p_ret->copy_on_write_ref.p_prev = p_this;

    } else {
        //Not allocated
        p_ret->attr = p_this->attr;
        p_ret->size = p_this->size;
        core_rtl_memset(&(p_this->mem_info), 0, sizeof(p_this->mem_info));
        p_ret->copy_on_write_ref.p_prev = NULL;
        p_ret->copy_on_write_ref.p_next = NULL;
    }

    //Set methods
    p_ret->swap = swap;
    p_ret->unswap = unswap;
    p_ret->set_attr = set_attr;
    p_ret->get_attr = get_attr;
    p_ret->get_size = get_size;
    p_ret->fork = fork;
    p_ret->copy_on_write = copy_on_write;
    p_ret->map = map;
    p_ret->unmap = unmap;
    p_ret->alloc = alloc;
    p_ret->is_alloced = is_alloced;

    return p_ret;
}

void copy_on_write(ppage_obj_t p_this, void* virt_addr, u32 attr)
{
    if(p_this->attr & PAGE_OBJ_SWAPPED) {
        p_this->unswap(p_this);
    }

    address_t new_phy_addr;

    if(p_this->copy_on_write_ref.p_prev == NULL
       && p_this->copy_on_write_ref.p_next == NULL) {
        p_this->attr &= ~PAGE_OBJ_COPY_ON_WRITE;

    } else {

        //Allocate memory
        while(hal_mmu_phymem_alloc((void**)(&new_phy_addr),
                                   SANDNIX_KERNEL_PAGE_SIZE,
                                   (p_this->attr & PAGE_OBJ_DMA) != 0,
                                   p_this->size / SANDNIX_KERNEL_PAGE_SIZE) != ESUCCESS) {
            //TODO:Swap
            NOT_SUPPORT;
        }

        //Copy memory
        u32 page_num = p_this->size / SANDNIX_KERNEL_PAGE_SIZE;

        for(u32 i = 0; i < page_num; i++) {
            hal_mmu_pg_tbl_set(0, page_copy_buf, MMU_PAGE_RW,
                               (void*)(new_phy_addr + i * SANDNIX_KERNEL_PAGE_SIZE));
            core_rtl_memcpy(page_copy_buf,
                            (void*)((address_t)virt_addr + i * SANDNIX_KERNEL_PAGE_SIZE),
                            SANDNIX_KERNEL_PAGE_SIZE);
        }

        //Set attributes
        p_this->mem_info.phy_mem_info.addr = new_phy_addr;
        copy_on_write_unref(p_this);
        p_this->attr &= ~PAGE_OBJ_COPY_ON_WRITE;
    }


    //Remap memory
    p_this->map(p_this, virt_addr, attr);

    return;
}

void map(ppage_obj_t p_this, void* virt_addr, u32 attr)
{
    //Check attribute
    if((p_this->attr & PAGE_OBJ_ALLOCATED) == 0) {
        PANIC(EINVAL, "Cannot map memory whitch is not allocated.");
    }

    //Get page attributes
    u32 mmu_attr = 0;

    if((attr & PAGE_BLOCK_WRITABLE)
       && !(p_this->attr & PAGE_OBJ_COPY_ON_WRITE)) {
        mmu_attr = MMU_PAGE_RW_NC;

    } else {
        mmu_attr = MMU_PAGE_RDONLY_NC;
    }

    if(p_this->attr & PAGE_OBJ_CAHCEABLE) {
        mmu_attr |= MMU_PAGE_CACHEABLE;
    }

    if(attr & PAGE_BLOCK_EXECUTABLE) {
        mmu_attr |= MMU_PAGE_EXECUTABLE;
    }

    //Map pages
    u32 page_num = p_this->size / SANDNIX_KERNEL_PAGE_SIZE;

    for(u32 i = 0; i < page_num; i++) {
        hal_mmu_pg_tbl_set(core_pm_get_crrnt_thread_id(),
                           virt_addr, mmu_attr,
                           (void*)(p_this->mem_info.phy_mem_info.addr
                                   + i * SANDNIX_KERNEL_PAGE_SIZE));
    }

    return;
}

void unmap(ppage_obj_t p_this, void* virt_addr)
{
    //Check attribute
    if((p_this->attr & PAGE_OBJ_ALLOCATED) == 0) {
        PANIC(EINVAL, "Cannot map memory whitch is not allocated.");
    }

    //Unmap pages
    u32 page_num = p_this->size / SANDNIX_KERNEL_PAGE_SIZE;

    for(u32 i = 0; i < page_num; i++) {
        hal_mmu_pg_tbl_set(core_pm_get_crrnt_thread_id(),
                           virt_addr, 0,
                           (void*)(p_this->mem_info.phy_mem_info.addr
                                   + i * SANDNIX_KERNEL_PAGE_SIZE));
    }

    return;
}

void alloc(ppage_obj_t p_this)
{
    //Check attribute
    if(p_this->attr & PAGE_OBJ_ALLOCATED) {
        PANIC(EINVAL, "Cannot allocate memoy for an allocated memory.");
    }

    void* new_phy_addr;

    while(hal_mmu_phymem_alloc(&new_phy_addr,
                               SANDNIX_KERNEL_PAGE_SIZE,
                               (p_this->attr & PAGE_OBJ_DMA) != 0,
                               p_this->size / SANDNIX_KERNEL_PAGE_SIZE) != ESUCCESS) {
        //TODO:Swap
        NOT_SUPPORT;
    }

    p_this->attr |= PAGE_OBJ_ALLOCATED;
    p_this->mem_info.phy_mem_info.addr = (address_t)new_phy_addr;

    return;
}

bool is_alloced(ppage_obj_t p_this)
{
    if(p_this->attr & PAGE_OBJ_ALLOCATED) {
        return true;

    } else {
        return false;
    }
}

void copy_on_write_unref(ppage_obj_t p_this)
{
    if(p_this->copy_on_write_ref.p_prev != NULL) {
        p_this->copy_on_write_ref.p_prev->copy_on_write_ref.p_next
            = p_this->copy_on_write_ref.p_next;
    }

    if(p_this->copy_on_write_ref.p_next != NULL) {
        p_this->copy_on_write_ref.p_next->copy_on_write_ref.p_prev
            = p_this->copy_on_write_ref.p_prev;
    }

    return;
}
