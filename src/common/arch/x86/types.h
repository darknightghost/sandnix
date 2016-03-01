/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

typedef	unsigned char			u8;
typedef unsigned short			u16;
typedef unsigned long			u32;
typedef unsigned long long		u64;

typedef signed char				s8;
typedef signed short			s16;
typedef signed long				s32;
typedef signed long long		s64;

typedef	u8		le8;
typedef	u16		le16;
typedef	u32		le32;
typedef	u64		le64;

typedef	u8		bool;
#define	true	1
#define	false	0

typedef u32		size_t;
typedef s32		ssize_t;

#define	NULL	((void*)0)
