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

#ifndef	EXCEPTION_H_INCLUDE
#define	EXCEPTION_H_INCLUDE

#include "../types.h"

#define	EXCEPTION_UNKNOW_EXCEPTION		0x00000000
#define	EXCEPTION_HEAP_CORRUPTION		0x00000001

void		panic(u32 reason);

#endif	//! EXCEPTION_H_INCLUDE
