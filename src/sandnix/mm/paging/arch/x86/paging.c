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

#include "paging.h"
#include "page_table.h"
#include "../../../../setup/setup.h"
#include "../../../../rtl/rtl.h"
#include "../../../../debug/debug.h"
#include "../../../../exceptions/exceptions.h"
#include "../../../../pm/pm.h"

void*				pdt_table[MAX_PROCESS_NUM];
static	u32			current_pdt;
static	spin_lock	mem_page_lock;
static	char		pdt_copy_buf[sizeof(pde) * 1024];

//memory allocation
static	void*	kernel_mem_reserve(u32 base, u32 num);
static	void*	usr_mem_reserve(u32 base, u32 num);
static	void*	kernel_mem_commit(u32 base, u32 num, bool dma_flag, u32 attr);
static	void*	usr_mem_commit(u32 base, u32 num, bool dma_flag, u32 attr);

static	bool	kernel_mem_uncommit(u32 base, u32 num);
static	bool	usr_mem_uncommit(u32 base, u32 num);
static	void	kernel_mem_unreserve(u32 base, u32 num);
static	void	usr_mem_unreserve(u32 base, u32 num);

//Page table
static	u32		get_free_pdt();
static	void	fork_kernel_pdt();
static	void	fork_usr_pdt(u32 src);
static	void	free_kernel_pdt(u32 id);
static	void	free_usr_pdt(u32 id);

static	ppte	get_pte(u32 pdt, u32 index);
static	void	unused_pde_recycle(bool is_kernel);

static	void	map_phy_addr(void* phy_addr);
static	void	sync_kernel_pdt();

static	void	switch_to_0();
static	void	switch_back();

void init_paging()
{
	ppde p_pde;
	u32 i;

	dbg_print("Initializing paging...\n");
	pm_init_spn_lock(&mem_page_lock);

	//Initialize PDT table
	rtl_memset(pdt_table, 0, MAX_PROCESS_NUM + sizeof(void*));
	pdt_table[0] = (void*)TMP_PDT_BASE;

	//Initialize PDT of process 0
	dbg_print("Initializing  page table of process 0...\n");

	for(i = 0, p_pde = (ppde)(TMP_PDT_BASE + VIRTUAL_ADDR_OFFSET);
	    i < 1024;
	    i++, p_pde++) {
		if(i * 4096 * 1024 < KERNEL_MEM_BASE) {
			p_pde->present = PG_NP;
			p_pde->page_table_base_addr = 0;
		}

		p_pde->read_write = PG_RW;
		p_pde->user_supervisor = PG_SUPERVISOR;
		p_pde->write_through = PG_WRITE_THROUGH;
		p_pde->cache_disabled = PG_ENCACHE;
		p_pde->accessed = 0;
		p_pde->reserved = 0;
		p_pde->page_size = PG_SIZE_4K;
		p_pde->global_page = 0;
		p_pde->avail = PG_NORMAL;
	}

	current_pdt = 0;

	//Refresh TLB
	REFRESH_TLB;

	return;
}

void* mm_virt_alloc(void* start_addr, size_t size, u32 options, u32 attr)
{
	u32 base;
	u32 num;
	void* ret;

	//Compute which kind of page to allocate and how many pages to allocate
	base = (u32)start_addr / 4096;
	num = ((u32)start_addr + size) / 4096 + (((u32)start_addr + size) % 4096 ? 1 : 0) - base;

	pm_acqr_spn_lock(&mem_page_lock);

	switch_to_0();
	ret = NULL;

	if(options & MEM_USER) {
		if(options & MEM_RESERVE) {
			ret = usr_mem_reserve(base, num);

			if(ret == NULL) {
				unused_pde_recycle(false);
				switch_back();
				pm_rls_spn_lock(&mem_page_lock);
				return NULL;
			}
		}

		if(options & MEM_COMMIT) {
			if(ret == NULL) {
				ret = usr_mem_commit(base,
				                     num,
				                     options & MEM_DMA ? true : false,
				                     attr);

			} else {
				ret = usr_mem_commit((u32)ret / 4096,
				                     num,
				                     options & MEM_DMA ? true : false,
				                     attr);
			}
		}

		REFRESH_TLB;
		switch_back();
		pm_rls_spn_lock(&mem_page_lock);
		return ret;

	} else {

		if(options & MEM_RESERVE) {
			ret = kernel_mem_reserve(base, num);

			if(ret == NULL) {
				unused_pde_recycle(true);
				switch_back();
				pm_rls_spn_lock(&mem_page_lock);
				return NULL;
			}
		}

		if(options & MEM_COMMIT) {
			if(ret == NULL) {
				ret = kernel_mem_commit(base,
				                        num,
				                        options & MEM_DMA ? true : false,
				                        attr);

			} else {
				ret = kernel_mem_commit((u32)ret / 4096,
				                        num,
				                        options & MEM_DMA ? true : false,
				                        attr);
			}
		}

		sync_kernel_pdt();
		REFRESH_TLB;
		switch_back();
		pm_rls_spn_lock(&mem_page_lock);
		return ret;
	}

	pm_rls_spn_lock(&mem_page_lock);
	return NULL;
}

void mm_virt_free(void* start_addr, size_t size, u32 options)
{
	u32 base;
	u32 num;

	//Compute which kind of page to free and how many pages to free
	base = (u32)start_addr / 4096;
	num = ((u32)start_addr + size) / 4096 + (((u32)start_addr + size) % 4096 ? 1 : 0) - base;

	pm_acqr_spn_lock(&mem_page_lock);

	switch_to_0();

	if(options & MEM_USER) {
		if(usr_mem_uncommit(base, num)) {

			if(options & MEM_RELEASE) {
				//Free the page
				usr_mem_unreserve(base, num);
				unused_pde_recycle(false);
			}
		}

	} else {

		if(kernel_mem_uncommit(base, num)) {

			if(options & MEM_RELEASE) {
				//Free the page
				kernel_mem_unreserve(base, num);
				unused_pde_recycle(true);
			}
		}

		sync_kernel_pdt();
	}

	REFRESH_TLB;
	switch_back();
	pm_rls_spn_lock(&mem_page_lock);

	return;
}

void* mm_virt_map(void* virt_addr, void* phy_addr)
{
	ppte p_pte;
	u32 base;

	base = (u32)virt_addr / 4096;

	pm_acqr_spn_lock(&mem_page_lock);
	switch_to_0();

	if(base >= KERNEL_MEM_BASE / 4096) {
		//Kernel mem
		p_pte = get_pte(0, base);

		if(p_pte->avail != PG_RESERVED) {
			switch_back();
			pm_rls_spn_lock(&mem_page_lock);
			return NULL;
		}

		p_pte->present = PG_P;
		p_pte->avail = PG_MAPPED;
		p_pte->global_page = 1;
		p_pte->page_base_addr = (u32)phy_addr >> 12;
		sync_kernel_pdt();

	} else {
		//User mem
		p_pte = get_pte(current_pdt, base);

		if(p_pte->avail != PG_RESERVED) {
			switch_back();
			pm_rls_spn_lock(&mem_page_lock);
			return NULL;
		}

		p_pte->present = PG_P;
		p_pte->avail = PG_MAPPED;
		p_pte->page_base_addr = (u32)phy_addr >> 12;
	}

	switch_back();
	REFRESH_TLB;
	pm_rls_spn_lock(&mem_page_lock);
	return (void*)(base * 4096);
}

void mm_virt_unmap(void* virt_addr)
{
	ppte p_pte;
	u32 base;

	base = (u32)virt_addr / 4096;

	pm_acqr_spn_lock(&mem_page_lock);
	switch_to_0();

	if(base >= KERNEL_MEM_BASE / 4096) {
		//Kernel mem
		p_pte = get_pte(0, base);

		if(p_pte->avail != PG_MAPPED) {
			switch_back();
			pm_rls_spn_lock(&mem_page_lock);
			return;
		}

		p_pte->present = PG_NP;
		p_pte->avail = PG_RESERVED;
		p_pte->global_page = 0;
		sync_kernel_pdt();

	} else {
		//User mem
		p_pte = get_pte(current_pdt, base);

		if(p_pte->avail != PG_MAPPED) {
			switch_back();
			pm_rls_spn_lock(&mem_page_lock);
			return;
		}

		p_pte->present = PG_NP;
		p_pte->avail = PG_RESERVED;
	}

	switch_back();
	REFRESH_TLB;
	pm_rls_spn_lock(&mem_page_lock);
	return;
}

u32 mm_pg_tbl_fork(u32 parent)
{
	//TODO:Fork page tables
	return 0;
}

void mm_pg_tbl_free(u32 id)
{
	//TODO:Free page tables
	return;
}

void mm_pg_tbl_switch(u32 id)
{

	if(pdt_table[id] == NULL) {
		excpt_panic(EXCEPTION_ILLEGAL_PDT,
		            "An illegal id of page directory has been tied to switch to,the id is %p.",
		            id);
	}

	pm_acqr_spn_lock(&mem_page_lock);

	current_pdt = id;

	//Load CR3
	__asm__ __volatile__(
	    "movl	%0,%%eax\n\t"
	    "andl	$0xFFFFF000,%%eax\n\t"
	    "orl	$0x008,%%eax\n\t"
	    "movl	%%eax,%%cr3\n\t"
	    ::"m"(pdt_table[id]));

	pm_rls_spn_lock(&mem_page_lock);

	return;
}

void mm_pg_tbl_usr_spc_clear(u32 id)
{
	//TODO:Clear user space
}

//Status
void mm_get_info(pmem_info p_info)
{
	//TODO:Return memory info
}

void* kernel_mem_reserve(u32 base, u32 num)
{
	ppte p_pte;
	u32 i;
	u32 page_count;

	//Check arguments
	if(base != 0
	   && base * 4096 < KERNEL_MEM_BASE) {
		return NULL;
	}

	if(base == 0) {
		//Test if there are enough free pages
		for(i = 0, p_pte = get_pte(0, base);
		    i < num;
		    i++, p_pte = get_pte(0, base + i)) {
			if(p_pte->present != PG_NP
			   || p_pte->avail != PG_NORMAL) {
				return NULL;
			}
		}

		//Allocate pages
		for(i = 0, p_pte = get_pte(0, base);
		    i < num;
		    i++, p_pte = get_pte(0, base + i)) {
			p_pte->read_write = PG_RW;
			p_pte->user_supervisor = PG_SUPERVISOR;
			p_pte->write_through = PG_WRITE_THROUGH;
			p_pte->cache_disabled = 0;
			p_pte->accessed = 0;
			p_pte->dirty = 0;
			p_pte->page_table_attr_index = 0;
			p_pte->global_page = 1;
			p_pte->avail = PG_RESERVED;
			p_pte->page_base_addr = 0;

			return (void*)(base * 4096);
		}

	} else {
		page_count = 0;

		for(base = TMP_PAGED_MEM_SIZE / 4096;
		    base < 1024 * 1024;
		    base++) {
			p_pte = get_pte(0, base);

			//Count free pages
			if(p_pte->present == PG_NP
			   || p_pte->avail == PG_NORMAL) {
				page_count++;

			} else {
				page_count = 0;
			}

			if(page_count >= num) {
				base -= (page_count - 1);

				//Allocate pages
				for(i = 0, p_pte = get_pte(0, base);
				    i < num;
				    i++, p_pte = get_pte(0, base + i)) {
					p_pte->read_write = PG_RW;
					p_pte->user_supervisor = PG_SUPERVISOR;
					p_pte->write_through = PG_WRITE_THROUGH;
					p_pte->cache_disabled = 0;
					p_pte->accessed = 0;
					p_pte->dirty = 0;
					p_pte->page_table_attr_index = 0;
					p_pte->global_page = 0;
					p_pte->avail = PG_RESERVED;
					p_pte->page_base_addr = 0;

					return (void*)(base * 4096);
				}
			}
		}
	}

	return NULL;
}

void* usr_mem_reserve(u32 base, u32 num)
{
	ppte p_pte;
	u32 i;
	u32 page_count;

	//Check arguments
	if(base != 0
	   && base * 4096 >= KERNEL_MEM_BASE) {
		return NULL;
	}


	if(base == 0) {
		//Test if there are enough free pages
		for(i = 0, p_pte = get_pte(current_pdt, base);
		    i < num;
		    i++, p_pte = get_pte(current_pdt, base + i)) {
			if(p_pte->present != PG_NP
			   || p_pte->avail != PG_NORMAL) {
				return NULL;
			}
		}

		//Allocate pages
		for(i = 0, p_pte = get_pte(current_pdt, base);
		    i < num;
		    i++, p_pte = get_pte(current_pdt, base + i)) {
			p_pte->read_write = PG_RW;
			p_pte->user_supervisor = PG_USER;
			p_pte->write_through = PG_WRITE_THROUGH;
			p_pte->cache_disabled = 0;
			p_pte->accessed = 0;
			p_pte->dirty = 0;
			p_pte->page_table_attr_index = 0;
			p_pte->global_page = 0;
			p_pte->avail = PG_RESERVED;
			p_pte->page_base_addr = 0;

			return (void*)(base * 4096);
		}

	} else {
		page_count = 0;

		for(base = 1;
		    base < 1024 * 1024;
		    base++) {
			p_pte = get_pte(current_pdt, base);

			//Count free pages
			if(p_pte->present == PG_NP
			   || p_pte->avail == PG_NORMAL) {
				page_count++;

			} else {
				page_count = 0;
			}

			if(page_count >= num) {
				base -= (page_count - 1);

				//Allocate pages
				for(i = 0, p_pte = get_pte(current_pdt, base);
				    i < num;
				    i++, p_pte = get_pte(current_pdt, base + i)) {
					p_pte->read_write = PG_RW;
					p_pte->user_supervisor = PG_USER;
					p_pte->write_through = PG_WRITE_THROUGH;
					p_pte->cache_disabled = 0;
					p_pte->accessed = 0;
					p_pte->dirty = 0;
					p_pte->page_table_attr_index = 0;
					p_pte->global_page = 0;
					p_pte->avail = PG_RESERVED;
					p_pte->page_base_addr = 0;

					return (void*)(base * 4096);
				}
			}
		}
	}

	return NULL;
}

void* kernel_mem_commit(u32 base, u32 num, bool dma_flag, u32 attr)
{
	ppte p_pte;
	u32 i;
	void* phy_mem_addr;

	//Allocate physical memory
	if(dma_flag) {
		if(!alloc_dma_physcl_page(NULL, num, &phy_mem_addr)) {
			return NULL;
		}

	} else {
		phy_mem_addr = alloc_physcl_page(NULL, num);

		if(phy_mem_addr == NULL) {
			//Swap
			//Try again
			phy_mem_addr = alloc_physcl_page(NULL, 1);

			if(phy_mem_addr == NULL) {
				return NULL;
			}
		}
	}

	//Commit pages
	for(i = 0, p_pte = get_pte(0, base);
	    i < num;
	    i++, p_pte = get_pte(0, base + i)) {
		if(p_pte->avail == PG_RESERVED
		   && p_pte->present == PG_NP) {
			p_pte->page_base_addr = ((u32)phy_mem_addr + i * 4096) >> 12;
			p_pte->present = PG_P;

			if(attr & PAGE_WRITEABLE) {
				p_pte->read_write = PG_RW;

			} else {
				p_pte->read_write = PG_RDONLY;
			}

		} else {
			excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
			            "A page whitch is not reserved has been tried to commit.the address of the page is %p.\n",
			            (base + i) * 4096);
		}
	}

	return (void*)(base * 4096);
}

void* usr_mem_commit(u32 base, u32 num, bool dma_flag, u32 attr)
{
	ppte p_pte;
	u32 i;
	void* phy_mem_addr;

	//Allocate physical memory
	if(dma_flag) {
		if(!alloc_dma_physcl_page(NULL , num, &phy_mem_addr)) {
			return NULL;
		}

	} else {
		phy_mem_addr = alloc_physcl_page(NULL, num);

		if(phy_mem_addr == NULL) {
			//Swap
			//Try again
			phy_mem_addr = alloc_physcl_page(NULL, 1);

			if(phy_mem_addr == NULL) {
				return NULL;
			}
		}
	}

	//Commit pages
	for(i = 0, p_pte = get_pte(current_pdt, base);
	    i < num;
	    i++, p_pte = get_pte(current_pdt, base + i)) {
		if(p_pte->avail == PG_RESERVED
		   && p_pte->present == PG_NP) {
			p_pte->page_base_addr = ((u32)phy_mem_addr + i * 4096) >> 12;
			p_pte->present = PG_P;

			if(attr & PAGE_WRITEABLE) {
				p_pte->read_write = PG_RW;

			} else {
				p_pte->read_write = PG_RDONLY;
			}

		} else {
			excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
			            "A page whitch is not reserved has been tried to commit.the address of the page is %p.\n",
			            (base + i) * 4096);
		}
	}

	return (void*)(base * 4096);
}

void map_phy_addr(void* phy_addr)
{
	ppte p_pte;

	p_pte = (ppte)PT_MAPPING_PAGE;
	p_pte->present = PG_P;
	p_pte->read_write = PG_RW;
	p_pte->user_supervisor = PG_SUPERVISOR;
	p_pte->write_through = PG_WRITE_THROUGH;
	p_pte->cache_disabled = 0;
	p_pte->accessed = 0;
	p_pte->dirty = 0;
	p_pte->page_table_attr_index = 0;
	p_pte->global_page = 0;
	p_pte->avail = PG_NORMAL;
	p_pte->page_base_addr = (u32)phy_addr >> 12;

	__asm__ __volatile__(
	    "movl	%%cr3,%%eax\n\t"
	    "movl	%%eax,%%cr3\n\t"
	    ::);

	return;
}

void sync_kernel_pdt()
{
	u32 i;

	for(i = 1; i < MAX_PROCESS_NUM; i++) {
		if(pdt_table[i] != NULL) {
			rtl_memcpy((void*)(PT_MAPPING_ADDR + sizeof(pde) * (KERNEL_MEM_BASE / 4096 / 1024)),
			           (void*)(TMP_PDT_BASE + VIRTUAL_ADDR_OFFSET + sizeof(pde) * (KERNEL_MEM_BASE / 4096 / 1024)),
			           4096 - sizeof(pde) * (KERNEL_MEM_BASE / 4096 / 1024));
		}
	}

	return;
}

u32 get_free_pdt()
{
	u32 i;

	for(i = 0; i < MAX_PROCESS_NUM; i++) {
		if(pdt_table[i] == NULL) {
			return i;
		}
	}

	return 0;
}

void fork_kernel_pdt()
{
	ppde p_pde;
}

void fork_usr_pdt(u32 src)
{

}

void free_kernel_pdt(u32 id)
{

}

void free_usr_pdt(u32 id)
{

}

ppte get_pte(u32 pdt, u32 index)
{
	ppde p_pde;
	ppte p_pte;
	void* phy_mem_addr;

	if(index >= KERNEL_MEM_BASE / 4096) {
		p_pde = (ppde)(TMP_PDT_BASE + VIRTUAL_ADDR_OFFSET);

	} else {
		map_phy_addr(pdt_table[pdt]);
		p_pde = (ppde)(PT_MAPPING_ADDR);
	}

	p_pde += index / 1024;

	if(p_pde->present == PG_NP
	   && p_pde->present == PG_NORMAL) {
		//Allocate physical memory and initialize PDE
		phy_mem_addr = alloc_physcl_page(NULL, 1);

		if(phy_mem_addr == NULL) {
			//TODO:Swap
			//Try again
			phy_mem_addr = alloc_physcl_page(NULL, 1);

			if(phy_mem_addr == NULL) {
				excpt_panic(EXCEPTION_RESOURCE_DEPLETED,
				            "Physical memory depleted and no pages can be swapped!\n");
			}
		}

		p_pde->page_table_base_addr = (u32)phy_mem_addr >> 12;
		p_pde->present = PG_P;
		map_phy_addr((void*)(p_pde->page_table_base_addr << 12));
		rtl_memset((void*)PT_MAPPING_ADDR, 0, 4096);

	} else if(p_pde->present == PG_NP
	          && p_pde->avail == PG_SWAPPED) {
		//TODO:Swap
	} else {
		map_phy_addr((void*)(p_pde->page_table_base_addr << 12));
	}

	p_pte = (ppte)(PT_MAPPING_ADDR);
	p_pte += index % 1024;

	return p_pte;

}

void unused_pde_recycle(bool is_kernel)
{
	ppte p_pte;
	ppde p_pde;
	u32 i, j;
	bool recycle_flag;

	if(is_kernel) {

		for(i = KERNEL_MEM_BASE / 4096 / 1024, p_pde = (ppde)(TMP_PDT_BASE + VIRTUAL_ADDR_OFFSET) + i;
		    i < 1024;
		    i++, p_pde++) {
			if(p_pde->present == PG_P) {
				map_phy_addr((void*)(p_pde->page_table_base_addr << 12));

				//Check if the pde should be recycled
				for(j = 0, p_pte = (ppte)(PT_MAPPING_ADDR), recycle_flag = true;
				    j < 1024;
				    j++, p_pte++) {
					if(p_pte->present == PG_P
					   || p_pte->avail != PG_NORMAL) {
						recycle_flag = false;
						break;
					}
				}

				if(recycle_flag) {
					//Recycle
					free_physcl_page((void*)(p_pde->page_table_base_addr << 12), 1);
					p_pde->present = PG_NP;
				}
			}
		}

	} else {
		for(i = 0, p_pde = (ppde)(TMP_PDT_BASE + VIRTUAL_ADDR_OFFSET);
		    i < KERNEL_MEM_BASE / 1024 / 4096;
		    i++, p_pde++) {
			map_phy_addr(pdt_table[current_pdt]);

			if(p_pde->present == PG_P) {
				map_phy_addr((void*)(p_pde->page_table_base_addr << 12));

				//Check if the pde should be recycled
				for(j = 0, p_pte = (ppte)(PT_MAPPING_ADDR), recycle_flag = true;
				    j < 1024;
				    j++, p_pte++) {
					if(p_pte->present == PG_P
					   || p_pte->avail != PG_NORMAL) {
						recycle_flag = false;
						break;
					}
				}

				if(recycle_flag) {
					//Recycle
					free_physcl_page((void*)(p_pde->page_table_base_addr << 12), 1);
					p_pde->present = PG_NP;
				}
			}
		}
	}

	return;
}

bool kernel_mem_uncommit(u32 base, u32 num)
{
	ppte p_pte;
	u32 i;

	//Check arguments
	if(base * 4096 < KERNEL_MEM_BASE) {
		return false;
	}

	for(i = 0, p_pte = get_pte(0, base);
	    i < num;
	    i++, p_pte = get_pte(0, base + i)) {
		//Uncommit the page
		if(p_pte->present == PG_P
		   && (p_pte->avail == PG_NORMAL
		       || p_pte->avail == PG_COW_RW
		       || p_pte->avail == PG_COW_RDONLY)) {
			//The page is in physical memory
			free_physcl_page((void*)(p_pte->page_base_addr << 12), 1);
			p_pte->present = PG_NP;
			p_pte->avail = PG_RESERVED;
			p_pte->global_page = 0;

		} else if(p_pte->present == PG_NP
		          && p_pte->avail == PG_SWAPPED) {
			//The page is in swap
			//TODO:
			excpt_panic(EXCEPTION_UNKNOW, "MEM in swap,file:%s\nLine:%d\n",
			            __FILE__,
			            __LINE__);

		} else {
			excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
			            "You have tried to uncommit a page whitch is not commited,The virtual address is %p",
			            (base + i) * 4096);
		}
	}

	return false;
}

bool usr_mem_uncommit(u32 base, u32 num)
{
	ppte p_pte;
	u32 i;

	//Check arguments
	if(base * 4096 >= KERNEL_MEM_BASE) {
		return false;
	}

	for(i = 0, p_pte = get_pte(current_pdt, base);
	    i < num;
	    i++, p_pte = get_pte(current_pdt, base + i)) {
		//Uncommit the page
		if(p_pte->present == PG_P
		   && (p_pte->avail == PG_NORMAL
		       || p_pte->avail == PG_COW_RW
		       || p_pte->avail == PG_COW_RDONLY)) {
			//The page is in physical memory
			free_physcl_page((void*)(p_pte->page_base_addr << 12), 1);
			p_pte->present = PG_NP;
			p_pte->avail = PG_RESERVED;

		} else if(p_pte->present == PG_NP
		          && p_pte->avail == PG_SWAPPED) {
			//The page is in swap
			//TODO:
			excpt_panic(EXCEPTION_UNKNOW, "MEM in swap,file:%s\nLine:%d\n",
			            __FILE__,
			            __LINE__);

		} else {
			excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
			            "You have tried to uncommit a page whitch is not commited,The virtual address is %p",
			            (base + i) * 4096);
		}


	}

	return false;
}

void kernel_mem_unreserve(u32 base, u32 num)
{
	ppte p_pte;
	u32 i;

	//Check arguments
	if(base * 4096 < KERNEL_MEM_BASE) {
		return;
	}

	for(i = 0, p_pte = get_pte(0, base);
	    i < num;
	    i++, p_pte = get_pte(0, base + i)) {
		//Unreserve the page
		if(p_pte->present == PG_P
		   && p_pte->avail == PG_RESERVED) {
			p_pte->read_write = PG_RW;
			p_pte->user_supervisor = PG_SUPERVISOR;
			p_pte->write_through = PG_WRITE_THROUGH;
			p_pte->cache_disabled = 0;
			p_pte->accessed = 0;
			p_pte->dirty = 0;
			p_pte->page_table_attr_index = 0;
			p_pte->global_page = 0;
			p_pte->avail = PG_NORMAL;
			p_pte->page_base_addr = 0;

		} else {
			excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
			            "A page which is not PG_RESERVED has been tried to release.The address is %p.\n",
			            (base + i) * 1024);
		}
	}

	return;
}

void usr_mem_unreserve(u32 base, u32 num)
{
	ppte p_pte;
	u32 i;

	//Check arguments
	if(base * 4096 >= KERNEL_MEM_BASE) {
		return;
	}

	for(i = 0, p_pte = get_pte(current_pdt, base);
	    i < num;
	    i++, p_pte = get_pte(current_pdt, base + i)) {
		//Unreserve the page
		if(p_pte->present == PG_P
		   && p_pte->avail == PG_RESERVED) {
			p_pte->read_write = PG_RW;
			p_pte->user_supervisor = PG_USER;
			p_pte->write_through = PG_WRITE_THROUGH;
			p_pte->cache_disabled = 0;
			p_pte->accessed = 0;
			p_pte->dirty = 0;
			p_pte->page_table_attr_index = 0;
			p_pte->global_page = 0;
			p_pte->avail = PG_NORMAL;
			p_pte->page_base_addr = 0;

		} else {
			excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
			            "A page which is not PG_RESERVED has been tried to release.The address is %p.\n",
			            (base + i) * 1024);
		}
	}

	return;

}

void switch_to_0()
{
	//Load CR3
	__asm__ __volatile__(
	    "movl	%0,%%eax\n\t"
	    "andl	$0xFFFFF000,%%eax\n\t"
	    "orl	$0x008,%%eax\n\t"
	    "movl	%%eax,%%cr3\n\t"
	    ::"m"(pdt_table[0]));
	return;

}

void switch_back()
{
	//Load CR3
	__asm__ __volatile__(
	    "movl	%0,%%eax\n\t"
	    "andl	$0xFFFFF000,%%eax\n\t"
	    "orl	$0x008,%%eax\n\t"
	    "movl	%%eax,%%cr3\n\t"
	    ::"m"(pdt_table[current_pdt]));
	return;
}
