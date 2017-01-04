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

#pragma once
#include "../../../../../../common/common.h"
#include "../../../../core/rtl/rtl_defs.h"


void	analyse_bootloader_info(void* p_info);

//Get pyhsical memory map from bootloader
list_t	hal_init_get_boot_memory_map();

//Get physical address and size initialize ramdisk.
void	hal_init_get_initrd_addr(
    void** p_addr,
    size_t* p_size);

//Get kernel commandline
char*	hal_init_get_kernel_cmdline();
