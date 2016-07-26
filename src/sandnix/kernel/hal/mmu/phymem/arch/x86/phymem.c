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

void archecture_phyaddr_edit(plist_t p_phy_addr_list)
{
    pphysical_memory_info_t p_dma_info;

    p_dma_info = core_mm_heap_alloc(sizeof(physical_memory_info_t), NULL);

    p_dma_info->begin = (void*)0x00000000;
    p_dma_info->size = 4096 * 4096 * 4;
    p_dma_info->type = PHYMEM_DMA;

    core_rtl_list_insert_after(NULL, p_phy_addr_list, p_dma_info, NULL);
    return;
}
