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
#include "../../../../../core/rtl/rtl.h"
#include "../../../../init/init.h"
#include "../../../mmu.h"
#include "../../../../exception/exception.h"

void archecture_phyaddr_edit(plist_t p_phy_addr_list)
{
    pphysical_memory_info_t p_dma_info;
    plist_node_t p_node;

    p_dma_info = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);

    if(p_dma_info == NULL) {
        PANIC(ENOMEM,
              "Failed to allocate memory for physical memory information.");
    }

    p_dma_info->begin = 0x00000000;
    p_dma_info->size = 1024 * 1024 * 4;
    p_dma_info->type = PHYMEM_DMA;

    core_rtl_list_insert_after(NULL, p_phy_addr_list, p_dma_info, NULL);

    p_node = *p_phy_addr_list;

    while(p_node != NULL) {
        pphysical_memory_info_t p_memblk = (pphysical_memory_info_t)(
                                               p_node->p_item);

        if(p_memblk->begin >= 0x0000000100000000) {
            plist_node_t p_remove_node = p_node;
            p_node = core_rtl_list_next(p_node, p_phy_addr_list);
            core_rtl_list_remove(p_remove_node, p_phy_addr_list, NULL);
            continue;

        } else if(p_memblk->begin + p_memblk->size >= 0x0000000100000000) {
            p_memblk->size = 0x0000000100000000 - p_memblk->begin;
        }

        p_node = core_rtl_list_next(p_node, p_phy_addr_list);
    }


    return;
}
