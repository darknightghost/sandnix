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

#ifndef	SYSCALL_H_INCLUDE
#define	SYSCALL_H_INCLUDE

#include "../../common/common.h"

#define	SYSCALL_MAX		0xFFFF

#ifndef	_ASM
	typedef		void*		(*syscall_t)(void*);
	#define	SYSTEM_CALL(num,func,table)	((table)[(num)] = (syscall_t)(func))

	#include "ssudt/ssudt.h"
	#include "ssddt/ssddt.h"

	void		syscall_init();
#endif	//!	_ASM

#endif	//!	SYSCALL_H_INCLUDE
