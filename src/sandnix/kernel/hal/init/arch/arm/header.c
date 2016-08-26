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
#include "../../../../../headers/uboot/uboot.h"
#include "../../../mmu/mmu.h"
#include "../../../../core/rtl/rtl.h"
#include "../../../../core/mm/mm.h"
#include "../../../early_print/early_print.h"
#include "../../../exception/exception.h"

static	list_t		boot_mem_map;
static	void*		initrd_addr;
static	size_t		initrd_size;
static	char*		kernel_cmdline;

#if BOOTLOADER == UBOOT
void analyse_bootloader_info(void* p_info)
{
    puboot_tag_t p_tag;
    puboot_tag_t p_begin_tag;
    pphysical_memory_info_t p_phymem;

    core_rtl_list_init(&boot_mem_map);
    hal_early_print_puts("\nAnalysing boot informations...\n");
    hal_early_print_puts("Booting protocol : u-boot\n");

    //Check first tag
    p_begin_tag = (puboot_tag_t)hal_mmu_add_early_paging_addr(p_info,
                  MMU_PAGE_RW);
    p_tag = p_begin_tag;

    if(p_tag->tag_header.tag != ATAG_CORE) {
        hal_exception_panic(EKERNELARG,
                            "Illegal type of first u-boot argument tag.\n");
    }

    p_tag = (puboot_tag_t)(
                (address_t)p_tag + p_tag->tag_header.size * sizeof(u32));

    //Analyse tags
    while(1) {
        switch(p_tag->tag_header.tag) {
            case ATAG_NONE:
                goto _end;

            case ATAG_CORE:
                break;

            case ATAG_MEM:
                p_phymem = core_mm_heap_alloc(sizeof(physical_memory_info_t),
                                              NULL);

                if(p_phymem == NULL) {
                    hal_exception_panic(ENOMEM, "Failed to allocate memory for "
                                        "physical memory information.");
                }

                p_phymem->begin = p_tag->data.tag_mem.start;
                p_phymem->size = p_tag->data.tag_mem.size;
                p_phymem->type = PHYMEM_AVAILABLE;

                if(core_rtl_list_insert_after(NULL, &boot_mem_map,
                                              p_phymem, NULL) == NULL) {
                    hal_exception_panic(ENOMEM, "Failed to allocate memory for "
                                        "physical memory information.");
                }

                break;

            case ATAG_INITRD:
            case ATAG_INITRD2:
                initrd_addr = (void*)(p_tag->data.tag_initrd.start);
                initrd_size = p_tag->data.tag_initrd.size;
                break;

            case ATAG_CMDLINE:
                kernel_cmdline = (char*)((address_t)(p_tag->data.tag_cmdline.cmdline)
                                         - (address_t)p_begin_tag + (address_t)p_info);
                break;
        }

        p_tag = (puboot_tag_t)(
                    (address_t)p_tag + p_tag->tag_header.size * sizeof(u32));
    }

_end:
    hal_early_print_printf("Initrd address : %p.\n", initrd_addr);
    hal_early_print_printf("Initrd size : %p.\n", initrd_size);
    hal_early_print_printf("Kernel cmdline address : %p.\n", kernel_cmdline);
    return;
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
#endif
