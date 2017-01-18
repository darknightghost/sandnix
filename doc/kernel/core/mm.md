#mm moudule
##Paging
###Summary
This module does not operarte the page table of platform directly. It keeps an abstract page table which records the relationship among virtual address, `page_block_t`s and `page_object_t`s. `page_object_t`s use the interfaces of mmu module to operate the page table in order to separate policy from mechanism.
Never call the methods of page_object_t out of this module.
###Importent data structures
1.`proc_pg_tbl_t`(`src/sandnix/kernel/core/mm/paging/paging_defs.h`)
User space page table of process. It refers to a number of `page_block_t`.
2.`page_block_t`(`src/sandnix/kernel/core/mm/paging/paging_defs.h`)
A block of pages. It has 3 status : free, allocated, commited. When it is commited, it refers to a `page_obj_t`.
3.`page_obj_t`(`src/sandnix/kernel/core/mm/paging/page_object_defs.h`)
Object of memory. It refers to physical memory pages or swapped pages. And it operates the page table of platform
###Interfaces
####`void core_mm_init();`
#####Description
Initialize this module.
#####Parameters
None.
#####Return value
None.

####`void core_mm_core_init(u32 cpuid);`
#####Description
Initialize datastructures of a new cpu core.
#####Parameters
* cpuid
ID of new cpu core.

#####Return value
None

####`void core_mm_core_release(u32 cpuid);`
#####Description
Releas the datastructures of a cpu core.
#####Parameters
* cpuid
ID of new cpu core.

#####Return value
None

####`void core_mm_switch_to(u32 index);`
#####Description
Switch to a page table.
#####Parameters
* index
Index of the page table.

#####Return value
None.

####`u32 core_mm_get_current_pg_tbl_index();`
#####Description
Get the index of the page table of current process.
#####Parameters
None
#####Return value
The index of the page thable.

####`u32 core_mm_pg_tbl_fork(u32 index);`
#####Description
Fork the userspace page table of current page table.
#####Parameters
* index
The index of source page table. If index is illegal, EINVAL will be raised.

#####Return value
The index of new page table.

####`void core_mm_pg_tbl_release(u32 index);`
#####Description
Release a page table. If index is illegal, EINVAL will be raised.
#####Parameters
* index
The index of page table to release.

#####Return value
None

####`void* core_mm_pg_alloc(void* base_addr, size_t size, u32 options);`
#####Description
Alloc pages.
#####Parameters
* base_addr
The base address of new memory. If NULL, The kernel will choose an address.

* size
Size of pages. It will be aligned to the size of a page.

* options
Options of the operation:
PAGE_OPTION_COMMIT
PAGE_OPTION_WRITABLE
PAGR_OPTION_EXECUTABLE
PAGE_OPTION_SWAPPABLE
PAGE_OPTION_DMA
PAGE_OPTION_KERNEL


#####Return value
If succeeded, the beginging adddress of the pages will be returned.
If failed, NULL will be returned and errno will be set.

####`void core_mm_pg_free(void* base_addr);`
#####Description
Free pages.
#####Parameters
* base_addr
The value returned by `core_mm_pg_alloc`.

#####Return value
None.

####`ppage_obj_t core_mm_get_pg_obj(void** p_base_addr, void* addr);`
#####Description
Get page object of the page.
#####Parameters
* p_base_addr
Return the base virtual address of the pages. Must not be NULL.

* addr
Virtuall address whereyou want to get the page object.

#####Return value
Pointer to the page object. If failed, returns NULL and errno will be set.

####`void* core_mm_map(void* addr, ppage_obj_t p_page_obj, u32 options);`
#####Description
Map page object.
#####Parameters
* addr
The base address to map the page object. If NULL, The kernel will choose an address.
If not NULL, the status of th address must be uncommited.

* p_page_obj
The page object to map.

* options
Options of the operation:
PAGE_OPTION_WRITABLE
PAGR_OPTION_EXECUTABLE
PAGE_OPTION_KERNEL

#####Return value
If succeeded, the beginging adddress of the pages will be returned.
If failed, NULL will be returned and errno will be set.

####`void core_mm_commit(void* addr);`
#####Description
Commit pages.
#####Parameters
* addr
Address to commit. If the address has been commited, EINVAL will be raised.

#####Return value
None.

####`void core_mm_uncommit(void* addr);`
#####Description
Uncommit pages.
#####Parameters
* addr
Address to uncommit. If the address has not been commited, EINVAL will be raised.

#####Return value
None

####`u32	core_mm_get_pg_attr(void* address);`
#####Description
Get the attribute of the page.
#####Parameters
* address
Address to get attribute.

#####Return value
The attribute of page.