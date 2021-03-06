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

#include "./ipi_defs.h"

#include "../../context/context_defs.h"

#include "../../../../core/pm/pm_defs.h"
#include "../../../../core/rtl/rtl_defs.h"

#include "./ipi_arg_obj.h"

#define	MODULE_NAME hal_cpu
void PRIVATE(cpu_ipi_init)();
void PRIVATE(cpu_ipi_core_init)();
void PRIVATE(cpu_ipi_core_release)();

//Send IPI
void hal_cpu_send_IPI(s32 index, u32 type, void* p_args);

//Regist IPI handler
ipi_hndlr_t hal_cpu_regist_IPI_hndlr(u32 type, ipi_hndlr_t hndlr);

#undef MODULE_NAME
