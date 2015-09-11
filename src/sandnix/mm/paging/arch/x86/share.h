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

#ifndef	SHARE_H_INCLUDE
#define	SHARE_H_INCLUDE

#include "../../../../../common/common.h"

typedef	struct {
	u32			ref_count;
	void*		phy_addr;
	size_t		size;
} pmo_t, *ppmo_t;


#endif	//!	SHARE_H_INCLUDE
