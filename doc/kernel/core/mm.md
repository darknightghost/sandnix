#mm moudule
<script>
alert("aaaa");
</script>
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
`void core_mm_init();`

`void core_mm_core_init(u32 cpuid);`

`void core_mm_core_release(u32 cpuid);`

`void core_mm_switch_to(u32 index);`

`u32 core_mm_get_current_pg_tbl_index();`

`u32 core_mm_pg_tbl_fork(u32 id);`

`void core_mm_pg_tbl_release(u32 index);`

`void* core_mm_pg_alloc(void* base_addr, size_t size, u32 options);`

`void core_mm_pg_free(void* base_addr);`

`ppage_obj_t core_mm_get_pg_obj(void** p_base_addr, void* addr);`

`void* core_mm_map(void* addr, ppage_obj_t p_page_obj, u32 options);`

`void core_mm_unmap(void* addr, ppage_obj_t p_page_obj);`