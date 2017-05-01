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
#include "./cond_defs.h"
#include "../../../mm/mm_defs.h"

#define	MODULE_NAME		core_pm

void		core_pm_cond_init(pcond_t p_cond, pmutex_t p_mutex);
kstatus_t	core_pm_cond_wait(pcond_t p_cond, s32 millisec_timeout);
kstatus_t	core_pm_cond_multi_wait(pcond_t* conds, size_t num,
                                    u32* p_ret_indexs, size_t* p_ret_num,
                                    s32 millisec_timeout, pheap_t heap);
kstatus_t	core_pm_cond_signal(pcond_t p_cond, bool broadcast);
void		core_pm_cond_destroy(pcond_t p_cond);

#undef	MODULE_NAME
