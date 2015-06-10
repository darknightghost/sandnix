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

#ifndef	SPINLOCK_H_INCLUDE
#define	SPINLOCK_H_INCLUDE

#ifdef X86
	#include "../../../../../common/arch/x86/types.h"
#endif

typedef	struct {
	u32		owner;
	u32		next;
	u32		int_level;
} spin_lock, *pspin_lock;

#endif	//!	SPINLOCK_H_INCLUDE
