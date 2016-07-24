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
#include "../../../early_print/early_print.h"
#include "../../../exception/exception.h"

static	list_t		boot_mem_map;
static	void*		initrd_addr;
static	size_t		initrd_size;
static	char*		kernel_cmdline;

#if BOOTLOADER == MULTIBOOT2
void analyse_bootloader_info(void* p_info)
{
    pmultiboot_tag_t p_tag;
    u32 boot_info_size;
    pmultiboot_tag_module_t p_module_info;
    pmultiboot_tag_string_t p_cmdline_info;
    pmultiboot_tag_mmap_t p_mmap_info;

    //Initialize variables
    core_rtl_list_init(&boot_mem_map);
    initrd_addr = NULL;
    initrd_size = 0;
    kernel_cmdline = NULL;

    hal_early_print_puts("Analysing boot informations...\n");
    hal_early_print_puts("Booting protocol : Multiboot2\n");

    boot_info_size = *((u32*)p_info);
    p_tag = (pmultiboot_tag_t)((address_t)p_info + 8);


    while((address_t)p_tag - (address_t)p_info > boot_info_size) {
        p_tag = (pmultiboot_tag_t)((address_t)p_tag + p_tag->size);

        if(p_tag->type == MULTIBOOT_TAG_TYPE_END) {
            break;
        }

        switch(p_tag->type) {
            case MULTIBOOT_TAG_TYPE_MODULE:
                break;

            case MULTIBOOT_TAG_TYPE_CMDLINE:
                break;

            case MULTIBOOT_TAG_TYPE_MMAP:
                break;
        }

        //Make p_tag 8 bytes aligned
        if((address_t)p_tag % 8 != 0) {
            p_tag = (pmultiboot_tag_t)((address_t)p_tag + (8 - (address_t)p_tag % 8));
        }
    }

    if(core_rtl_list_empty(&boot_mem_map)) {
        hal_exception_panic(EKERNELARG, "Mmemory map is missing.\n");
    }

    if(initrd_addr == NULL) {
        hal_exception_panic(EKERNELARG, "Initialize ramdisk is missing.\n");
    }

    if(kernel_cmdline == NULL) {
        hal_exception_panic(EKERNELARG, "Kernel command line is missing.\n");
    }
}
#endif
