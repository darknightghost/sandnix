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

#include "./uboot.h"

.section	.text 
.global		_start

ih_magic:
	.long	0x56190527

ih_hcrc:
	.long	0

ih_time:
	.long	0

ih_size:
	.long	0

ih_load:
	.long	0

ih_ep:
	.long	0

ih_dcrc:
	.long	0

ih_os:
	.byte	IH_OS_LINUX

ih_arch:
	.byte	0

ih_type:
	.byte	0

ih_comp:
	.byte	IH_COMP_NONE

ih_name:
	.asciz	"SANDNIX"

.rept		IH_NMLEN - 8
	.byte	0
.endr

