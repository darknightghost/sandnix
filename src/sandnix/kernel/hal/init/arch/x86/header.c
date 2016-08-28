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

#include "header.h"
#include "../../../../../headers/multiboot2/multiboot2.h"
#include "../../../mmu/mmu.h"
#include "../../../../core/rtl/rtl.h"
#include "../../../../core/mm/mm.h"
#include "../../../early_print/early_print.h"
#include "../../../exception/exception.h"

static	list_t		boot_mem_map;
static	void*		initrd_addr;
static	size_t		initrd_size;
static	char*		kernel_cmdline;

#if BOOTLOADER == MULTIBOOT2
static	void	analyse_map_info(pmultiboot_tag_mmap_t p_mmap_tag);

void analyse_bootloader_info(void* p_info)
{
    pmultiboot_tag_t p_tag;
    u32 boot_info_size;
    pmultiboot_tag_module_t p_module_info;
    pmultiboot_tag_string_t p_cmdline_info;
    pmultiboot_tag_mmap_t p_mmap_info;

    p_info = hal_mmu_add_early_paging_addr(p_info, MMU_PAGE_RW);

    //Initialize variables
    core_rtl_list_init(&boot_mem_map);
    initrd_addr = NULL;
    initrd_size = 0;
    kernel_cmdline = NULL;

    hal_early_print_puts("\nAnalysing boot informations...\n");
    hal_early_print_puts("Booting protocol : Multiboot2\n");

    boot_info_size = *((u32*)p_info);
    p_tag = (pmultiboot_tag_t)((address_t)p_info + 8);

    while((address_t)p_tag - (address_t)p_info < boot_info_size) {

        if(p_tag->type == MULTIBOOT_TAG_TYPE_END) {
            break;
        }

        switch(p_tag->type) {
            case MULTIBOOT_TAG_TYPE_MODULE:
                p_module_info = (pmultiboot_tag_module_t)p_tag;
                initrd_addr = (void*)(p_module_info->mod_start);
                initrd_size = (address_t)(p_module_info->mod_end)
                              - (address_t)(p_module_info->mod_start);
                break;

            case MULTIBOOT_TAG_TYPE_CMDLINE:
                p_cmdline_info = (pmultiboot_tag_string_t)p_tag;
                kernel_cmdline = p_cmdline_info->string;
                kernel_cmdline = (char*)((address_t)kernel_cmdline - KERNEL_MEM_BASE);
                break;

            case MULTIBOOT_TAG_TYPE_MMAP:
                p_mmap_info = (pmultiboot_tag_mmap_t)p_tag;
                analyse_map_info(p_mmap_info);
                break;
        }


        //Make p_tag 8 bytes aligned
        p_tag = (pmultiboot_tag_t)((address_t)p_tag + p_tag->size);

        if((address_t)p_tag % 8 != 0) {
            p_tag = (pmultiboot_tag_t)((address_t)p_tag + (8 - (address_t)p_tag % 8));
        }
    }

    if(core_rtl_list_empty(&boot_mem_map)) {
        PANIC(EKERNELARG, "Memory map is missing.\n");
    }

    if(initrd_addr == NULL) {
        PANIC(EKERNELARG, "Initialize ramdisk is missing.\n");
    }

    if(kernel_cmdline == NULL) {
        PANIC(EKERNELARG, "Kernel command line is missing.\n");
    }

    hal_early_print_printf("Initrd address : %p.\n", initrd_addr);
    hal_early_print_printf("Initrd size : %p.\n", initrd_size);
    hal_early_print_printf("Kernel cmdline address : %p.\n", kernel_cmdline);
}

list_t hal_init_get_boot_memory_map()
{
    return boot_mem_map;
}

void hal_init_get_initrd_addr(void** p_addr, size_t* p_size)
{
    *p_addr = initrd_addr;
    *p_size = initrd_size;
    return;
}

char* hal_init_get_kernel_cmdline()
{
    return kernel_cmdline;
}

void analyse_map_info(pmultiboot_tag_mmap_t p_mmap_tag)
{
    pmultiboot_mmap_entry_t p_entry;
    pphysical_memory_info_t p_phymem_info;

    for(p_entry = p_mmap_tag->entries;
        (address_t)p_entry < (address_t)p_mmap_tag + p_mmap_tag->size;
        p_entry = (pmultiboot_mmap_entry_t)((address_t)p_entry + p_mmap_tag->entry_size)) {
        p_phymem_info = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);
        p_phymem_info->begin = (u64)(p_entry->addr);
        p_phymem_info->size = (u64)(p_entry->len);

        switch(p_entry->type) {
            case MULTIBOOT_MEMORY_AVAILABLE:
                p_phymem_info->type = PHYMEM_AVAILABLE;
                break;

            case MULTIBOOT_MEMORY_RESERVED:
                p_phymem_info->type = PHYMEM_RESERVED;
                break;

            case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
                p_phymem_info->type = PHYMEM_RESERVED;
                break;

            case MULTIBOOT_MEMORY_NVS:
                p_phymem_info->type = PHYMEM_RESERVED;
                break;

            case MULTIBOOT_MEMORY_BADRAM:
                p_phymem_info->type = PHYMEM_BAD;
                break;

            default:
                p_phymem_info->type = PHYMEM_RESERVED;
        }

        core_rtl_list_insert_after(NULL, &boot_mem_map, p_phymem_info, NULL);
    }

    return;
}
#endif
