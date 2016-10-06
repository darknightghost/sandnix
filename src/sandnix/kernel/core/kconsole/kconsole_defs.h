/*
    Copyright 2016,王思远 <darknightghost.cn@gmail.com>

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

#include "../../../../common/common.h"

//Print level
//Kernel panic
#define PRINT_LEVEL_PANIC	0x00000001

//Error
#define PRINT_LEVEL_ERR		0x00000002

//Alert
#define PRINT_LEVEL_WARNING	0x00000003

//Information
#define PRINT_LEVEL_INFO	0x00000004

//Debug info
#define PRINT_LEVEL_DEBUG	0x00000005
