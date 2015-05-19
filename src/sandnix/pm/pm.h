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
#endif	//X86

//schedule.c
void		pm_schedule();
u32			pm_create_thrd();
void		pm_terminate_thrd(u32 thread_id);
void		pm_suspend_thrd(u32 thread_id);
void		pm_resume_thrd(u32 thread_id);
void		pm_sleep(u32 ms);
u32			pm_get_crrnt_thrd_id();
void		pm_task_schedule();

#endif	//!	PM_H_INCLUDE

