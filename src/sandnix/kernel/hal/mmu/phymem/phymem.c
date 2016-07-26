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

#include "phymem.h"
#include "../../init/init.h"

static	map_t	phymem_info_map;

void phymem_init()
{
    list_t info_list;

    //Get physical memeory information from bootloader.
    info_list = hal_init_get_boot_memory_map();

    archecture_phyaddr_edit(&info_list);
}

kstatus_t hal_mmu_phymem_alloc(
    void** p_addr,		//Start address
    bool is_dma,		//DMA page
    size_t page_num);	//Num

void hal_mmu_phymem_free(
    void* addr,			//Address
    size_t page_num);	//Num

size_t hal_mmu_get_phymem_info(
    pphysical_memory_info_t p_buf,	//Pointer to buffer
    size_t size);
