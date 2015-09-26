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

#ifndef	MUTEX_H_INCLUDE
#define	MUTEX_H_INCLUDE

#include "../pm.h"
#include "../spinlock/arch/x86/spinlock.h"
#include "../../rtl/rtl.h"

typedef	struct {
	spinlock_t	lock;
	bool		is_acquired;
	u32			current_thread;
	u32			next_thread;
	list_t		acquire_list;
} mutex_t, *pmutex_t;


#endif	//!	MUTEX_H_INCLUDE
