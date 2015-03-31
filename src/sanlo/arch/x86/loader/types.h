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

#ifndef	TYPES_H_INCLUDE
#define	TYPES_H_INCLUDE

#define	LOADER_OFFSET			0x00030000

#define	GET_REAL_ADDR(p)		(((char*)(p)) + LOADER_OFFSET)
#define	GET_REAL_VARIABLE(p)	(*((char*)(&p) + LOADER_OFFSET))

#define	NULL			((void*)0)

#define	true			1
#define	false			0

typedef	unsigned int	size_t;

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned long	u32;
typedef signed char		s8;
typedef short			s16;
typedef long			s32;

typedef unsigned char	le8;
typedef unsigned short	le16;
typedef unsigned long	le32;

typedef	int				bool;


#endif	//! TYPES_H_INCLUDE
