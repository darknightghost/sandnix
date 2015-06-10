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
#include "../../../../io/io.h"

spin_lock				mem_lock;

static	void*			pdt_index_table[MAX_PROCESS_NUM];
static	u32				current_pdt;
static	u32				prev_pdt;
static	char			pdt_copy_buf[sizeof(pde) * 1024];
static	int_hndlr_info	pf_hndlr_info;

//memory allocation
static	void*	kernel_mem_reserve(u32 base, u32 num);
static	void*	usr_mem_reserve(u32 base, u32 num);
static	void*	kernel_mem_commit(u32 base, u32 num, bool dma_flag, u32 attr);
static	void*	usr_mem_commit(u32 base, u32 num, bool dma_flag, u32 attr);

static	void	kernel_mem_uncommit(u32 base, u32 num);
static	void	usr_mem_uncommit(u32 base, u32 num);
static	void	kernel_mem_unreserve(u32 base, u32 num);
static	void	usr_mem_unreserve(u32 base, u32 num);

//Page table
static	u32		get_free_pdt();
static	void	fork_pdt(u32 dest, u32 src);
static	void	fork_user_pages(void* pt_phy_addr, void* src_pt_phy_addr);
static	void	free_usr_pdt(u32 id);

static	ppte	get_pte(u32 pdt, u32 index);
static	void	unused_pde_recycle(bool is_kernel);

static	void	map_phy_addr(void* phy_addr);
static	void	sync_kernel_pdt();

static	void	switch_to_0();
static	void	switch_back();

static	bool	pf_hndlr(u32 int_num, u32 thread_id, u32 err_code);

void init_paging()
{
	ppde p_pde;
	u32 i;

	dbg_print("Initializing paging...\n");
	pm_init_spn_lock(&mem_lock);

	//Initialize PDT table
	rtl_memset(pdt_index_table, 0, MAX_PROCESS_NUM + sizeof(void*));
	pdt_index_table[0] = (void*)TMP_PDT_BASE;

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
	prev_pdt = 0;

	//Refresh TLB
	REFRESH_TLB;

	//Regist #PF handler
	dbg_print("Registing #PF handler...\n");
	pf_hndlr_info.func = pf_hndlr;
	io_reg_int_hndlr(INT_PF, &pf_hndlr_info);

	return;
}

bool pf_hndlr(u32 int_num, u32 thread_id, u32 err_code)
{
	ppde p_pde;
	ppte p_pte;
	u32 err_addr;
	void* new_mem;
	void* pt_addr;
	pf_err_code err = *(ppf_err_code)(&err_code);


	if(mem_lock.owner != mem_lock.next) {
		excpt_panic(EXCEPTION_UNKNOW,
		            "#PF occured in mm!\n");
	}

	__asm__ __volatile__(
	    "movl	%%cr2,%0\n\t"
	    :"=r"(err_addr)
	    :);

	if(err.present == 0) {
		//The page does not present
		map_phy_addr(pdt_index_table[prev_pdt]);
		p_pde = (ppde)PT_MAPPING_ADDR + err_addr / 1024 / 4096;;

		if(p_pde->present == PG_NP) {
			return false;
		}

		map_phy_addr((void*)(p_pde->page_table_base_addr << 12));
		p_pte = (ppte)PT_MAPPING_ADDR + err_addr % (1024 * 4096) / 4096;

		switch(p_pte->avail) {
		case PG_SWAPPED:
			//TODO:Swap
			excpt_panic(EXCEPTION_UNKNOW, "MEM in swap,file:%s\nLine:%d\n",
			            __FILE__,
			            __LINE__);
			__asm__ __volatile__(
			    "movl	%%cr3,%%eax\n\t"
			    "movl	%%eax,%%cr3\n\t"
			    ::);
			return true;

		default:
			return false;

		}

	} else if(err.read_write == 1) {

		//The page is not writeable
		map_phy_addr(pdt_index_table[prev_pdt]);
		p_pde = (ppde)PT_MAPPING_ADDR + err_addr / 1024 / 4096;
		pt_addr = (void*)(p_pde->page_table_base_addr << 12);
		map_phy_addr(pt_addr);
		p_pte = (ppte)PT_MAPPING_ADDR + err_addr % (1024 * 4096) / 4096;

		switch(p_pte->avail) {
		case PG_CP_ON_W_RW:
			if(get_ref_count((void*)(p_pte->page_base_addr << 12)) > 1) {
				//Copy the page
				new_mem = alloc_physcl_page(NULL, 1);

				if(new_mem == NULL) {
					//TODO:Swap
					excpt_panic(EXCEPTION_UNKNOW, "Not enough memory!\n");
				}

				map_phy_addr((void*)(p_pte->page_base_addr << 12));
				rtl_memcpy(pdt_copy_buf,
				           (void*)PT_MAPPING_ADDR,
				           4096);
				map_phy_addr(new_mem);
				rtl_memcpy((void*)PT_MAPPING_ADDR,
				           pdt_copy_buf,
				           4096);
				map_phy_addr(pt_addr);
				free_physcl_page((void*)(p_pte->page_base_addr << 12), 1);
				p_pte->avail = PG_NORMAL;
				p_pte->read_write = PG_RW;
				p_pte->page_base_addr = (u32)new_mem >> 12;

			} else {
				p_pte->read_write = PG_RW;
				p_pte->avail = PG_NORMAL;
			}

			__asm__ __volatile__(
			    "movl	%%cr3,%%eax\n\t"
			    "movl	%%eax,%%cr3\n\t"
			    ::);
			return true;

		default:
			return false;
		}
	}

	return false;
}

void* mm_virt_alloc(void* start_addr, size_t size, u32 options, u32 attr)
{
	u32 base;
	u32 num;
	void* ret;

	//Compute which kind of page to allocate and how many pages to allocate
	base = (u32)start_addr / 4096;
	num = ((u32)start_addr + size) / 4096 + (((u32)start_addr + size) % 4096 ? 1 : 0) - base;

	pm_acqr_spn_lock(&mem_lock);

	switch_to_0();
	ret = NULL;

	if(options & MEM_USER) {
		if(options & MEM_RESERVE) {
			ret = usr_mem_reserve(base, num);

			if(ret == NULL) {
				unused_pde_recycle(false);
				switch_back();
				pm_rls_spn_lock(&mem_lock);
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
		pm_rls_spn_lock(&mem_lock);
		return ret;

	} else {

		if(options & MEM_RESERVE) {
			ret = kernel_mem_reserve(base, num);

			if(ret == NULL) {
				unused_pde_recycle(true);
				switch_back();
				pm_rls_spn_lock(&mem_lock);
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
		pm_rls_spn_lock(&mem_lock);
		return ret;
	}

	pm_rls_spn_lock(&mem_lock);
	return NULL;
}

void mm_virt_free(void* start_addr, size_t size, u32 options)
{
	u32 base;
	u32 num;

	//Compute which kind of page to free and how many pages to free
	base = (u32)start_addr / 4096;
	num = ((u32)start_addr + size) / 4096 + (((u32)start_addr + size) % 4096 ? 1 : 0) - base;

	pm_acqr_spn_lock(&mem_lock);

	switch_to_0();

	if(options & MEM_USER) {
		if(options & MEM_UNCOMMIT) {
			usr_mem_uncommit(base, num);
		}

		if(options & MEM_RELEASE) {
			//Free the page
			usr_mem_unreserve(base, num);
			unused_pde_recycle(false);
		}

	} else {

		if(options & MEM_UNCOMMIT) {
			kernel_mem_uncommit(base, num);
		}

		if(options & MEM_RELEASE) {
			//Free the page
			kernel_mem_unreserve(base, num);
			unused_pde_recycle(true);
		}

		sync_kernel_pdt();
	}

	REFRESH_TLB;
	switch_back();
	pm_rls_spn_lock(&mem_lock);

	return;
}

void* mm_virt_map(void* virt_addr, void* phy_addr)
{
	ppte p_pte;
	u32 base;

	base = (u32)virt_addr / 4096;

	pm_acqr_spn_lock(&mem_lock);
	switch_to_0();

	if(base >= KERNEL_MEM_BASE / 4096) {
		//Kernel mem
		p_pte = get_pte(0, base);

		if(p_pte->avail != PG_RESERVED) {
			switch_back();
			pm_rls_spn_lock(&mem_lock);
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
			pm_rls_spn_lock(&mem_lock);
			return NULL;
		}

		p_pte->present = PG_P;
		p_pte->avail = PG_MAPPED;
		p_pte->page_base_addr = (u32)phy_addr >> 12;
	}

	increase_physcl_page_ref(phy_addr, 1);
	switch_back();
	REFRESH_TLB;
	pm_rls_spn_lock(&mem_lock);
	return (void*)(base * 4096);
}

void mm_virt_unmap(void* virt_addr)
{
	ppte p_pte;
	u32 base;

	base = (u32)virt_addr / 4096;

	pm_acqr_spn_lock(&mem_lock);
	switch_to_0();

	if(base >= KERNEL_MEM_BASE / 4096) {
		//Kernel mem
		p_pte = get_pte(0, base);

		if(p_pte->avail != PG_MAPPED) {
			switch_back();
			pm_rls_spn_lock(&mem_lock);
			return;
		}

		p_pte->present = PG_NP;
		p_pte->avail = PG_RESERVED;
		p_pte->global_page = 0;
		free_physcl_page((void*)((u32)(p_pte->page_base_addr) << 12), 1);
		sync_kernel_pdt();

	} else {
		//User mem
		p_pte = get_pte(current_pdt, base);

		if(p_pte->avail != PG_MAPPED) {
			switch_back();
			pm_rls_spn_lock(&mem_lock);
			return;
		}

		p_pte->present = PG_NP;
		p_pte->avail = PG_RESERVED;
		free_physcl_page((void*)((u32)p_pte->page_base_addr << 12), 1);
	}

	switch_back();
	REFRESH_TLB;
	pm_rls_spn_lock(&mem_lock);
	return;
}

u32 mm_pg_tbl_fork(u32 parent)
{
	u32 new_id;

	pm_acqr_spn_lock(&mem_lock);
	switch_to_0();
	new_id = get_free_pdt();

	if(new_id != 0) {
		fork_pdt(new_id, parent);
	}

	switch_back();
	pm_rls_spn_lock(&mem_lock);
	return new_id;
}

void mm_pg_tbl_free(u32 id)
{
	pm_acqr_spn_lock(&mem_lock);
	switch_to_0();
	free_usr_pdt(id);
	free_physcl_page(pdt_index_table[id], 1);
	pdt_index_table[id] = NULL;
	switch_back();
	pm_rls_spn_lock(&mem_lock);
	return;
}

void mm_pg_tbl_switch(u32 id)
{

	if(pdt_index_table[id] == NULL) {
		excpt_panic(EXCEPTION_ILLEGAL_PDT,
		            "An illegal id of page directory has been tied to switch to,the id is %p.",
		            id);
	}

	pm_acqr_spn_lock(&mem_lock);

	prev_pdt = current_pdt;
	current_pdt = id;

	//Load CR3
	__asm__ __volatile__(
	    "movl	%0,%%eax\n\t"
	    "andl	$0xFFFFF000,%%eax\n\t"
	    "orl	$0x008,%%eax\n\t"
	    "movl	%%eax,%%cr3\n\t"
	    ::"m"(pdt_index_table[id]));

	pm_rls_spn_lock(&mem_lock);

	return;
}

void mm_pg_tbl_usr_spc_clear(u32 id)
{
	pm_acqr_spn_lock(&mem_lock);
	switch_to_0();
	free_usr_pdt(id);
	switch_back();
	pm_rls_spn_lock(&mem_lock);
	return;
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
		excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
		            "Address %p is not in kernel memspace!\n",
		            base * 4096);
	}

	if(base != 0) {
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
			p_pte->present = PG_NP;
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

		}

		return (void*)(base * 4096);

	} else {
		page_count = 0;

		for(base = (KERNEL_MEM_BASE + TMP_PAGED_MEM_SIZE) / 4096 + 1;
		    base < 1024 * 1024;
		    base++) {
			p_pte = get_pte(0, base);

			//Count free pages
			if(p_pte->present == PG_NP
			   && p_pte->avail == PG_NORMAL) {
				page_count++;

			} else {
				page_count = 0;
				continue;
			}

			if(page_count >= num) {
				base -= page_count - 1;

				//Allocate pages
				for(i = 0, p_pte = get_pte(0, base);
				    i < num;
				    i++, p_pte = get_pte(0, base + i)) {
					p_pte->present = PG_NP;
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

				}

				return (void*)(base * 4096);
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
		excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
		            "Address %p is not in user memspace!\n",
		            base * 4096);
	}

	if(base != 0) {
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

		}

		return (void*)(base * 4096);

	} else {
		page_count = 0;

		for(base = 1;
		    base < 1024 * 1024;
		    base++) {
			p_pte = get_pte(current_pdt, base);

			//Count free pages
			if(p_pte->present == PG_NP
			   && p_pte->avail == PG_NORMAL) {
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

				}

				return (void*)(base * 4096);
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
			//TODO:Swap
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
			p_pte->avail = PG_NORMAL;

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
			//TODO:Swap
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
			p_pte->avail = PG_NORMAL;

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
		if(pdt_index_table[i] != NULL) {
			map_phy_addr(pdt_index_table[i]);
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
		if(pdt_index_table[i] == NULL) {
			return i;
		}
	}

	return 0;
}

void fork_pdt(u32 dest, u32 src)
{
	ppde p_pde;
	void* new_pdt;
	void* new_pt;

	//Copy kernel page directory table
	rtl_memcpy((void*)(pdt_copy_buf + KERNEL_MEM_BASE / 4096 / 1024 * sizeof(pde)),
	           (void*)(TMP_PDT_BASE + VIRTUAL_ADDR_OFFSET + KERNEL_MEM_BASE / 4096 / 1024 * sizeof(pde)),
	           4096 - KERNEL_MEM_BASE / 4096 / 1024 * sizeof(pde));

	//Copy user space page directory table
	map_phy_addr(pdt_index_table[src]);
	rtl_memcpy(pdt_copy_buf,
	           (void*)(PT_MAPPING_ADDR),
	           KERNEL_MEM_BASE / 4096 / 1024 * sizeof(pde));

	//Allocate memory
	new_pdt = alloc_physcl_page(NULL, 1);

	if(new_pdt == NULL) {
		//TODO:Swap
		excpt_panic(EXCEPTION_UNKNOW,
		            "Not enough memory!\n");
	}

	pdt_index_table[dest] = new_pdt;
	map_phy_addr(new_pdt);
	rtl_memcpy((void*)PT_MAPPING_ADDR,
	           pdt_copy_buf,
	           4096);

	//Copy user space page tables
	for(p_pde = (ppde)PT_MAPPING_ADDR;
	    p_pde < (ppde)(PT_MAPPING_ADDR) + KERNEL_MEM_BASE / 4096 / 1024;
	    p_pde++) {
		map_phy_addr(new_pdt);

		if(p_pde->present == PG_P
		   && p_pde->present == PG_NORMAL) {
			map_phy_addr((void*)((u32)(p_pde->page_table_base_addr << 12)));
			rtl_memcpy(pdt_copy_buf, (void*)PT_MAPPING_ADDR, 4096);

			//Allocate memory
			new_pt = alloc_physcl_page(NULL, 1);

			if(new_pt == NULL) {
				//TODO:Swap
				excpt_panic(EXCEPTION_UNKNOW,
				            "Not enough memory!\n");
			}

			map_phy_addr(new_pt);
			rtl_memcpy((void*)PT_MAPPING_ADDR, pdt_copy_buf, 4096);
			map_phy_addr(new_pdt);

			//Fork pages
			fork_user_pages(new_pt, (void*)((u32)(p_pde->page_table_base_addr) << 12));
			p_pde->page_table_base_addr = (u32)new_pt >> 12;

		} else if(p_pde->present == PG_NP
		          && p_pde->avail == PG_SWAPPED) {
			//TODO:Swap
			excpt_panic(EXCEPTION_UNKNOW, "MEM in swap,file:%s\nLine:%d\n",
			            __FILE__,
			            __LINE__);

		}
	}

	return;
}

void fork_user_pages(void* pt_phy_addr, void* src_pt_phy_addr)
{
	ppte p_pte;

	map_phy_addr(pt_phy_addr);

	for(p_pte = (ppte)PT_MAPPING_ADDR;
	    p_pte < (ppte)PT_MAPPING_ADDR + 1024;
	    p_pte++) {
		if(p_pte->present == PG_P) {
			switch(p_pte->avail) {
			case PG_NORMAL:
				increase_physcl_page_ref((void*)((u32)(p_pte->page_base_addr) << 12), 1);

				if(p_pte->read_write == PG_RW) {
					p_pte->avail = PG_CP_ON_W_RW;
					p_pte->read_write = PG_RDONLY;
					map_phy_addr(src_pt_phy_addr);
					p_pte->avail = PG_CP_ON_W_RW;
					p_pte->read_write = PG_RDONLY;
					map_phy_addr(pt_phy_addr);

				} else if(p_pte->read_write == PG_RDONLY) {
					p_pte->avail = PG_CP_ON_W_RDONLY;
					p_pte->read_write = PG_RDONLY;
					map_phy_addr(src_pt_phy_addr);
					p_pte->avail = PG_CP_ON_W_RDONLY;
					p_pte->read_write = PG_RDONLY;
					map_phy_addr(pt_phy_addr);

				}

				break;

			case PG_CP_ON_W_RW:
				increase_physcl_page_ref((void*)((u32)(p_pte->page_base_addr) << 12), 1);
				break;

			case PG_CP_ON_W_RDONLY:
				increase_physcl_page_ref((void*)((u32)(p_pte->page_base_addr) << 12), 1);
				break;

			}

		} else {
			switch(p_pte->avail) {
			case PG_SWAPPED:
				//TODO:Swap
				excpt_panic(EXCEPTION_UNKNOW, "MEM in swap,file:%s\nLine:%d\n",
				            __FILE__,
				            __LINE__);
				break;
			}
		}
	}

	return;
}

void free_usr_pdt(u32 id)
{
	ppde p_pde;
	ppte p_pte;

	//Check arguments
	if(pdt_index_table[id] == NULL) {
		excpt_panic(EXCEPTION_ILLEGAL_PDT,
		            "Some code tried to free a page table which id is %u,but it is unused.\n",
		            id);
	}

	//Copy pdt to buffer
	map_phy_addr(pdt_index_table[id]);
	rtl_memcpy(pdt_copy_buf, (void*)PT_MAPPING_ADDR, 4096);

	//Free userspace memory
	for(p_pde = (ppde)pdt_copy_buf;
	    p_pde < (ppde)pdt_copy_buf + KERNEL_MEM_BASE / 1024 / 4096;
	    p_pde++) {
		if(p_pde->present == PG_P
		   && p_pde->avail == PG_NORMAL) {
			//Free page table
			map_phy_addr((void*)((u32)(p_pde->page_table_base_addr) << 12));

			for(p_pte = (ppte)PT_MAPPING_ADDR;
			    p_pte < (ppte)PT_MAPPING_ADDR + 1024;
			    p_pte++) {
				if(p_pte->present == PG_NP
				   && p_pte->avail == PG_SWAPPED) {
					//TODO:Swap
					excpt_panic(EXCEPTION_UNKNOW, "MEM in swap,file:%s\nLine:%d\n",
					            __FILE__,
					            __LINE__);

				}

				if(p_pte->present == PG_P) {
					switch(p_pte->avail) {
					case PG_NORMAL:
					case PG_CP_ON_W_RW:
					case PG_CP_ON_W_RDONLY:
					case PG_MAPPED:
						free_physcl_page((void*)((u32)(p_pte->page_base_addr) << 12), 1);
						break;

					case PG_SHARED:
						//TODO:Clean the share
						//mm_pmo_unmap();
						excpt_panic(EXCEPTION_UNKNOW, "Shared mem:file:%s\nLine:%d\n",
						            __FILE__,
						            __LINE__);
						break;
					}
				}
			}

			p_pde->present = PG_NP;
			free_physcl_page((void*)((u32)(p_pde->page_table_base_addr) << 12), 1);
			p_pde->page_table_base_addr = 0;

		} else if(p_pde->present == PG_NP
		          && p_pde->avail == PG_SWAPPED) {
			//TODO:Swap
			excpt_panic(EXCEPTION_UNKNOW, "MEM in swap,file:%s\nLine:%d\n",
			            __FILE__,
			            __LINE__);
		}
	}

	//Copy back
	map_phy_addr(pdt_index_table[id]);
	rtl_memcpy((void*)PT_MAPPING_ADDR, pdt_copy_buf,  4096);
	return;
}

ppte get_pte(u32 pdt, u32 index)
{
	ppde p_pde;
	ppte p_pte;
	void* phy_mem_addr;

	if(index >= KERNEL_MEM_BASE / 4096) {
		p_pde = (ppde)(TMP_PDT_BASE + VIRTUAL_ADDR_OFFSET);

	} else {
		map_phy_addr(pdt_index_table[pdt]);
		p_pde = (ppde)(PT_MAPPING_ADDR);
	}

	p_pde += index / 1024;

	if(p_pde->present == PG_NP) {
		if(p_pde->avail == PG_NORMAL) {
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
			excpt_panic(EXCEPTION_UNKNOW, "MEM in swap,file:%s\nLine:%d\n",
			            __FILE__,
			            __LINE__);

		} else {
			excpt_panic(EXCEPTION_ILLEGAL_PDT,
			            "pdt_table has been broken!\n");
		}

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
			map_phy_addr(pdt_index_table[current_pdt]);

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

void kernel_mem_uncommit(u32 base, u32 num)
{
	ppte p_pte;
	u32 i;

	//Check arguments
	if(base * 4096 < KERNEL_MEM_BASE) {
		excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
		            "Address %p is not in kernel memspace!\n",
		            base * 4096);
	}

	for(i = 0, p_pte = get_pte(0, base);
	    i < num;
	    i++, p_pte = get_pte(0, base + i)) {

		//Uncommit the page
		if(p_pte->present == PG_P
		   && (p_pte->avail == PG_NORMAL
		       || p_pte->avail == PG_CP_ON_W_RW
		       || p_pte->avail == PG_CP_ON_W_RDONLY)) {
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

	return;
}

void usr_mem_uncommit(u32 base, u32 num)
{
	ppte p_pte;
	u32 i;

	//Check arguments
	if(base * 4096 >= KERNEL_MEM_BASE) {
		excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
		            "Address %p is not in user memspace!\n",
		            base * 4096);
	}


	for(i = 0, p_pte = get_pte(current_pdt, base);
	    i < num;
	    i++, p_pte = get_pte(current_pdt, base + i)) {
		//Uncommit the page
		if(p_pte->present == PG_P
		   && (p_pte->avail == PG_NORMAL
		       || p_pte->avail == PG_CP_ON_W_RW
		       || p_pte->avail == PG_CP_ON_W_RDONLY)) {
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

	return;
}

void kernel_mem_unreserve(u32 base, u32 num)
{
	ppte p_pte;
	u32 i;

	//Check arguments
	if(base * 4096 < KERNEL_MEM_BASE) {
		excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
		            "Address %p is not in kernel memspace!\n",
		            base * 4096);
	}

	for(i = 0, p_pte = get_pte(0, base);
	    i < num;
	    i++, p_pte = get_pte(0, base + i)) {
		//Unreserve the page
		if(p_pte->present == PG_NP
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
		excpt_panic(EXCEPTION_ILLEGAL_MEM_ADDR,
		            "Address %p is not in user memspace!\n",
		            base * 4096);
	}

	for(i = 0, p_pte = get_pte(current_pdt, base);
	    i < num;
	    i++, p_pte = get_pte(current_pdt, base + i)) {

		//Unreserve the page
		if(p_pte->present == PG_NP
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
	    ::"m"(pdt_index_table[0]));
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
	    ::"m"(pdt_index_table[current_pdt]));
	return;
}
