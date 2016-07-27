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

#include "../../phymem.h"
#include "../../../../../core/mm/mm.h"
#include "../../../../init/init.h"
#include "../../../mmu.h"

void archecture_phyaddr_edit(plist_t p_phy_addr_list)
{
    pphysical_memory_info_t p_dma_info;
    pphysical_memory_info_t p_system_info;
    void* initrd_addr;
    size_t initrd_size;
    address_t initrd_end;

    p_dma_info = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);

    p_dma_info->begin = 0x00000000;
    p_dma_info->size = 1024 * 1024 * 4;
    p_dma_info->type = PHYMEM_DMA;

    core_rtl_list_insert_after(NULL, p_phy_addr_list, p_dma_info, NULL);

    //Kernel
    p_system_info = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);

    p_system_info->begin = (u64)(address_t)MIN(kernel_header.code_start,
                           kernel_header.data_start) - KERNEL_MEM_BASE;
    p_system_info->size = (u64)(address_t)MAX(kernel_header.code_start
                          + kernel_header.code_size,
                          kernel_header.data_start + kernel_header.data_size)
                          - KERNEL_MEM_BASE - p_system_info->begin;
    p_system_info->type = PHYMEM_SYSTEM;

    core_rtl_list_insert_after(NULL, p_phy_addr_list, p_system_info, NULL);

    //Initrd
    hal_init_get_initrd_addr(&initrd_addr, &initrd_size);
    p_system_info = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);

    p_system_info->begin = (u64)((address_t)initrd_addr / 4096 * 4096);

    initrd_end = (address_t)initrd_addr + initrd_size;

    p_system_info->size = (u64)(initrd_end % 4096 > 0
                                ? (initrd_end / 4096 + 1) * 4096
                                : initrd_end) - p_system_info->begin;
    p_system_info->type = PHYMEM_SYSTEM;

    core_rtl_list_insert_after(NULL, p_phy_addr_list, p_system_info, NULL);

    return;
}
