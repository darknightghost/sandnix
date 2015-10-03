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

#include "../../../common/common.h"

#ifndef	INT_MSG_H_INCLUDE
#define	INT_MSG_H_INCLUDE

typedef	struct {
	u32					ref_count;
	u32					count;
	spinlock_t			count_lock;
	int_hndlr_info_t	info;
} int_msg_info_t, *pint_msg_info_t;

#endif	//!	INT_MSG_H_INCLUDE
