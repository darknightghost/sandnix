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

#define MODULE_NAME
#define	MK_P_NAME2(module_name, str, name)	str##_##module_name##_##name
#define	MK_P_NAME1(module_name, name)		MK_P_NAME2(module_name, private, name)
#define	PRIVATE(name)						MK_P_NAME1(MODULE_NAME, name)
#undef	MODULE_NAME
