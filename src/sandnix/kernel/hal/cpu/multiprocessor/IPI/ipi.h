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
#include "../../context/context.h"
#include "../../../../core/pm/pm.h"
#include "../../../../core/rtl/rtl.h"

#define	MAX_IPI_MSG_NUM		0x10

#define	IPI_TYPE_DIE			0x00
#define	IPI_TYPE_TLB_REFRESH	0x01

typedef	struct	_ipi_msg {
    u32		type;
    void*	p_args;
} ipi_msg_t, *pipi_msg_t;

typedef	struct	_ipi_queue {
    bool		initialized;
    queue_t		msg_queue;
    spnlck_t	lock;
} ipi_queue_t, *pipi_queue_t;

//void	ipi_hndlr(pcontext_t p_context, void* p_args);
typedef	void	(*ipi_hndlr_t)(pcontext_t, void*);

void cpu_ipi_init();
void cpu_ipi_core_init();
void cpu_ipi_core_release();

//Send IPI
void hal_cpu_send_IPI(s32 index, u32 type, void* p_args);

//Regist IPI handler
ipi_hndlr_t hal_cpu_regist_IPI_hndlr(u32 type, ipi_hndlr_t hndlr);
