/*
	Copyright 2015,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "share.h"
#include "../../../mm.h"

static	void		decrease_pmo_ref(ppmo p_pmo);

ppmo mm_pmo_create(size_t size)
{
	ppmo ret;

	//Allocate memory
	ret = mm_hp_alloc(sizeof(pmo_t), NULL);

	//Allocate physical memory
	ret->size = ((size % 4096 ? 1 : 0) + size / 4096) * 4096;
	ret->phy_addr = alloc_physcl_page(NULL, ret->size);

	if(ret->phy_addr == NULL) {
		mm_hp_free(ret, NULL);
		return NULL;
	}

	ret->ref_count = 1;

	return ret;
}

void mm_pmo_free(ppmo p_pmo)
{
	decrease_pmo_ref(p_pmo);
	return;
}

void* mm_pmo_map(void* address, ppmo p_pmo, bool is_user)
{
	void* ret;
	u32 flags;
	u32 phy_addr;
	u32 virt_addr;
	u32 i;

	flags = MEM_RESERVE;

	if(is_user) {
		flags = flags | MEM_USER;
	}

	//Allocate pages
	ret = mm_virt_alloc(address, p_pmo->size, flags, PAGE_WRITEABLE);

	if(ret == NULL) {
		return NULL;
	}

	//Map pages
	for(phy_addr = (u32)(p_pmo->phy_addr), virt_addr = (u32)(ret), i = 0;
	    i < p_pmo->size;
	    i += 4096, phy_addr += 4096, virt_addr += 4096) {
		if(mm_virt_map((void*)virt_addr, (void*)phy_addr) == NULL) {
			//Ummap pages
			for(virt_addr -= 4096; i > 0; virt_addr -= 4096, i -= 4096) {
				mm_virt_unmap((void*)virt_addr);
			}

			return NULL;
		}
	}

	(p_pmo->ref_count)++;

	return ret;
}

void mm_pmo_unmap(void* address, ppmo p_pmo)
{
	u32 offset;

	//Unmap pages
	for(offset = 0;
	    offset < p_pmo->size;
	    offset += 4096
	   ) {
		mm_virt_unmap((void*)((u32)address + offset));
	}

	//Decrease reference count
	decrease_pmo_ref(p_pmo);
	return;
}

void decrease_pmo_ref(ppmo p_pmo)
{
	(p_pmo->ref_count)--;

	if(p_pmo->ref_count == 0) {
		//Destroy the pmo_t
		free_physcl_page(p_pmo->phy_addr, p_pmo->size / 4096);
		mm_hp_free(p_pmo, NULL);
	}

	return;
}
