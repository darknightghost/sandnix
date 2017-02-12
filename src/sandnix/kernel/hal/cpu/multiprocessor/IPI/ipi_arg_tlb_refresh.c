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

#include "./ipi_arg_tlb_refresh.h"
#include "./ipi_defs.h"

pipi_arg_tlb_refresh_t ipi_arg_tlb_refresh(u32 src_cpu, u32 process_id,
        address_t virt_addr, u32 page_count)
{
    pipi_arg_tlb_refresh_t p_ret = (pipi_arg_tlb_refresh_t)ipi_arg_obj(
                                       sizeof(ipi_arg_tlb_refresh_t),
                                       CLASS_IPI_ARG(IPI_TYPE_TLB_REFRESH),
                                       src_cpu);

    if(p_ret != NULL) {
        p_ret->virt_addr = virt_addr;
        p_ret->page_count = page_count;
        p_ret->process_id = process_id;
    }

    return p_ret;
}
