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

#define	EXCEPTION_UNKNOW_EXCEPTION			0x00000000
#define	EXCEPTION_HEAP_CORRUPTION			0x00000001
#define	EXCEPTION_DE						0x00000002
#define	EXCEPTION_DB						0x00000003
#define	EXCEPTION_NMI						0x00000004
#define	EXCEPTION_BR						0x00000005
#define	EXCEPTION_UD						0x00000006
#define	EXCEPTION_NM						0x00000007
#define	EXCEPTION_DF						0x00000008
#define	EXCEPTION_FPU						0x00000009
#define	EXCEPTION_TS						0x0000000A
#define	EXCEPTION_NP						0x0000000B
#define	EXCEPTION_SS						0x0000000C
#define	EXCEPTION_GP						0x0000000D
#define	EXCEPTION_PF						0x0000000E
#define	EXCEPTION_RESERVED					0x0000000F
#define	EXCEPTION_MF						0x00000010
#define	EXCEPTION_AC						0x00000011
#define	EXCEPTION_MC						0x00000012
#define	EXCEPTION_XF						0x00000013
#define	EXCEPTION_NOT_ENOUGH_MEMORY			0x00000014
#define	EXCEPTION_NO_CONFIG_FILE			0x00000015
#define	EXCEPTION_UNEXPECT_CONFIG_FILE		0x00000016
#define	EXCEPTION_NO_KERNEL					0x00000017
#define	EXCEPTION_UNKNOW_KERNEL_FORMAT		0x00000018
#define	EXCEPTION_KERNEL_PARAMETER_TOO_LONG	0x00000019
#define	EXCEPTION_RAMDISK_TOO_LARGE			0x0000001A

#ifndef	_ASM
	void		panic(u32 reason);
#endif	//!	_ASM

#endif	//! EXCEPTION_H_INCLUDE
