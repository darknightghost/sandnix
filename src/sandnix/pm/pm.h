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

#ifndef	PM_H_INCLUDE
#define	PM_H_INCLUDE

#ifdef	X86
	#include "../../common/arch/x86/types.h"
	#include "arch/x86/spinlock.h"
	#include "arch/x86/schedule.h"
#endif	//X86


#define		MAX_PROCESS_NUM		65535

//schedule.c
void		pm_schedule();
u32			pm_create_thrd(void* entry);
void		pm_terminate_thrd(u32 thread_id);
void		pm_suspend_thrd(u32 thread_id);
void		pm_resume_thrd(u32 thread_id);
void		pm_sleep(u32 ms);
u32			pm_get_crrnt_thrd_id();
void		pm_task_schedule();

bool		pm_get_thread_context(u32 id, pcontext p_cntxt);
bool		pm_set_thread_context(u32 id, pcontext p_cntxt);

void		pm_init_spn_lock(pspin_lock p_lock);
void		pm_acqr_spn_lock(pspin_lock p_lock);
bool		pm_try_acqr_spn_lock(pspin_lock p_lock);
void		pm_rls_spn_lock(pspin_lock p_lock);

#endif	//!	PM_H_INCLUDE

