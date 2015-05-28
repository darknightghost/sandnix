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

#ifndef	ERR_H_INCLUDE
#define	ERR_H_INCLUDE

#define	EXCEPTION_UNKNOW						0x00000000
#define	EXCEPTION_UNHANDLED_DE					0x00000001
#define	EXCEPTION_UNHANDLED_DB					0x00000002
#define	EXCEPTION_UNHANDLED_BP					0x00000003
#define	EXCEPTION_UNHANDLED_OF					0x00000004
#define	EXCEPTION_UNHANDLED_BR					0x00000005
#define	EXCEPTION_UNHANDLED_UD					0x00000006
#define	EXCEPTION_UNHANDLED_NM					0x00000007
#define	EXCEPTION_UNHANDLED_DF					0x00000008
#define	EXCEPTION_UNHANDLED_FPU					0x00000009
#define	EXCEPTION_UNHANDLED_TS					0x0000000A
#define	EXCEPTION_UNHANDLED_NP					0x0000000B
#define	EXCEPTION_UNHANDLED_SS					0x0000000C
#define	EXCEPTION_UNHANDLED_GP					0x0000000D
#define	EXCEPTION_UNHANDLED_PF					0x0000000E
#define	EXCEPTION_UNHANDLED_MF					0x0000000F
#define	EXCEPTION_UNHANDLED_AC					0x00000010
#define	EXCEPTION_UNHANDLED_MC					0x00000011
#define	EXCEPTION_UNHANDLED_XF					0x00000012
#define	EXCEPTION_INT_LEVEL_ERROR				0x00000013
#define	EXCEPTION_INT_NUM_TOO_LARGE				0x00000014
#define	EXCEPTION_BUF_OVERFLOW					0x00000015
#define	EXCEPTION_HEAP_CORRUPTION				0x00000016
#define	EXCEPTION_ILLEGAL_HEAP_ADDR				0x00000017
#define	EXCEPTION_UNSPECIFIED_ROOT_PARTITION	0x00000018
#define	EXCEPTION_DOUBLE_FAULT					0x00000019
#define	EXCEPTION_ILLEGAL_MEM_ADDR				0x0000001A
#define	EXCEPTION_ILLEGAL_PDT					0x0000001B
#define	EXPECTION_RESOURCE_DEPLETED				0x0000001C

#endif	//ERR_H_INCLUDE
