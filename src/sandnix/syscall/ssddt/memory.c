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

#include "ssddt.h"
#include "ssddt_funcs.h"
#include "../../rtl/rtl.h"
#include "../../vfs/vfs.h"
#include "../../msg/msg.h"
#include "../../mm/mm.h"
#include "../../pm/pm.h"
#include "../../exceptions/exceptions.h"

void* ssddt_virt_alloc(va_list p_args)
{
	//Agruments
	void* addr;
	size_t size;
	u32 options;
	u32 attr;

	//Variables
	void* ret;

	//Get args
	addr = va_arg(p_args, void*);
	size = va_arg(p_args, size_t);
	options = va_arg(p_args, u32);
	attr = va_arg(p_args, u32);

	//Check arguments
	if((options | MEM_UNCOMMIT)
	   || (options | MEM_RELEASE)) {
		pm_set_errno(EINVAL);
		return NULL;
	}

	if(options | MEM_RESERVE) {
		if(mm_virt_test(addr, size, PG_STAT_RESERVE, false)) {
			pm_set_errno(EINVAL);
			return NULL;

		} else {
			if(!mm_virt_test(addr, size, PG_STAT_RESERVE, true)) {
				pm_set_errno(EINVAL);
				return NULL;
			}

			if(!(options | MEM_COMMIT)) {
				pm_set_errno(EINVAL);
				return NULL;
			}
		}
	}

	options = options | MEM_USER;

	ret = mm_virt_alloc(addr, size, options, attr);

	if(ret == NULL) {
		pm_set_errno(ENOMEM);

	} else {
		pm_set_errno(ESUCCESS);
	}

	return ret;
}

void ssddt_virt_free(va_list p_args)
{
	//Agruments
	void* start_addr;
	size_t size;
	u32 options;

	//Variables

	//Get args
	start_addr = va_arg(p_args, void*);
	size = va_arg(p_args, size_t);
	options = va_arg(p_args, u32);

	//Check arguments
	if((options | MEM_COMMIT) || (options | MEM_RESERVE)) {
		pm_set_errno(EINVAL);
		return;
	}

	if(option | MEM_UNCOMMIT) {
		if(!mm_virt_test(start_addr, size, PG_STAT_COMMIT, true)) {
			pm_set_errno(EINVAL);
			return;
		}

	} else {
		if(mm_virt_test(start_addr, size, PG_STAT_COMMIT, false)) {
			pm_set_errno(EINVAL);
			return;
		}

		if((option | MEM_RESERVE)) {
			pm_set_errno(EINVAL);
			return;
		}

		if(!mm_virt_test(start_addr, size, PG_STAT_RESERVE, true)) {
			pm_set_errno(EINVAL);
			return;
		}
	}

	option = option | MEM_USER;
	mm_virt_free(start_addr, size, options);
	pm_set_errno(ESUCCESS);
	return;
}

void* ssddt_map_pmo(va_list p_args)
{
	//Agruments
	void* address;
	ppmo_t p_pmo;

	//Variables

	//Get args
	address = va_arg(p_args, void*);
	p_pmo = va_arg(p_args, ppmo_t);

	return mm_pmo_map(address, p_pmo, true);
}

void ssddt_unmap_pmo(va_list p_args)
{
	//Agruments
	void* address;
	ppmo_t p_pmo;

	//Variables

	//Get args
	address = va_arg(p_args, void*);
	p_pmo = va_arg(p_args, ppmo_t);

	mm_pmo_unmap(address, p_pmo);
	return;
}

void* ssddt_map_reserv_mem(va_list p_args)
{
	//Agruments
	void* virt_addr;
	void* phy_addr;

	//Variables

	//Get args
	virt_addr = va_arg(p_args, void*);
	phy_addr = va_arg(p_args, void*);

	//Check arguments
	if((!mm_virt_test(virt_addr, 1, PG_STAT_RESERVE, true))
	   || mm_virt_test(virt_addr, 1, PG_STAT_COMMIT, true)) {
		pm_set_errno(EINVAL);
		return EINVAL;
	}

	return mm_virt_map(virt_addr, phy_addr);
}

void ssddt_umap_reserv_mem(va_list p_args)
{
	//Agruments
	void* virt_addr;

	//Variables

	//Get args
	virt_addr = va_arg(p_args, void*);

	mm_virt_unmap(virt_addr);
	return;
}
