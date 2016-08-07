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

#include "../../../../../phymem.h"
#include "../../../../../../../../core/mm/mm.h"
#include "../../../../../../../init/init.h"
#include "../../../../../../mmu.h"
#include "../../../../../../../exception/exception.h"

void archecture_phyaddr_edit(plist_t p_phy_addr_list)
{
    pphysical_memory_info_t p_mem_info;

#define	ADD_MEM_BLOCK(_begin, _size, _type) { \
        p_mem_info = core_mm_heap_alloc(sizeof(physical_memory_info_t), \
                                        NULL); \
        if(p_mem_info == NULL) { \
            hal_exception_panic(ENOMEM, \
                                "Failed to allocate memory for physical memory information."); \
        } \
        p_mem_info->begin = (_begin); \
        p_mem_info->size = (_size); \
        p_mem_info->type = (_type); \
        core_rtl_list_insert_after(NULL, p_phy_addr_list, p_mem_info, NULL); \
    }

    //iROM.
    ADD_MEM_BLOCK(0x00000000, 64 * 1024, PHYMEM_RESERVED);

    //iROM(mirror of 0x0 tp 0x1000).
    ADD_MEM_BLOCK(0x02000000, 64 * 1024, PHYMEM_RESERVED);

    //iRAM.
    ADD_MEM_BLOCK(0x02020000, 256 * 1024, PHYMEM_RESERVED);

    //Data memory or general purpose of Samsung Reconfigurable Processor SRP.
    ADD_MEM_BLOCK(0x03000000, 128 * 1024, PHYMEM_RESERVED);

    //I-cache or general purpose of SRP.
    ADD_MEM_BLOCK(0x03020000, 64 * 1024, PHYMEM_RESERVED);

    //Configuration memory(write only) of SRP
    ADD_MEM_BLOCK(0x03030000, 36 * 1024, PHYMEM_RESERVED);

    //AudioSS's SFR region
    ADD_MEM_BLOCK(0x03810000, 0x03830000 - 0x03810000, PHYMEM_RESERVED);

    //Bank0 of Static Read only Memory Controller (SMC) (16-bit only).
    ADD_MEM_BLOCK(0x04000000, 16 * 1024 * 1024, PHYMEM_RESERVED);

    //Bank1 of SMC
    ADD_MEM_BLOCK(0x05000000, 16 * 1024 * 1024 , PHYMEM_RESERVED);

    //Bank2 of SMC
    ADD_MEM_BLOCK(0x06000000, 16 * 1024 * 1024 , PHYMEM_RESERVED);

    //Bank3 of SMC
    ADD_MEM_BLOCK(0x08000000, 16 * 1024 * 1024 , PHYMEM_RESERVED);

    //Reserved
    ADD_MEM_BLOCK(0x08000000, 0x0CD00000 - 0x08000000 , PHYMEM_RESERVED);

    //SFR region of Nand Flash Controller (NFCON)
    ADD_MEM_BLOCK(0x0CE00000, 0x0D000000 , PHYMEM_RESERVED);

    //SFR region
    ADD_MEM_BLOCK(0x10000000, 0x14000000 - 0x10000000, PHYMEM_RESERVED);

    return;
}
