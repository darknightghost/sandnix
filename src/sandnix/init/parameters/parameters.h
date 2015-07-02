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

#ifndef	PARAMETERS_H_INCLUDE
#define	PARAMETERS_H_INCLUDE

#include "../../../common/arch/x86/types.h"
#include "../../../common/arch/x86/kernel_image.h"

typedef	struct _param_info {
	char*	root_partition;
	char*	driver_init;
	char*	init;
} param_info, *pparam_info;

extern	param_info		kernel_param;

void	get_kernel_param();

#endif	//!	PARAMETERS_H_INCLUDE
