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

#include "../../init.h"

.global		_start

.section	.text
.align		8
kernel_header:
	//magic
	.long	KERNEL_HEADER_MAGIC
	//code_start
	.long	0
	//code_size
	.long	0
	//data_start
	.long	0
	//data_size
	.long	0
	//header_size
	.long	_kernel_header_end - kernel_header
	//checksum
	.long	0x100000000 - ((KERNEL_HEADER_MAGIC + (_kernel_header_end - kernel_header)) & 0xFFFFFFFF)
_kernel_header_end:
	.long	0

_start:
	mov		r0, #1
	mov		r0, #2
	mov		r0, #3
	mov		r0, #4
	mov		r0, #5
	mov		r0, #6
	mov		r0, #7
	mov		r0, #8
	mov		r0, #9
	mov		r0, #1
	mov		r0, #2
	mov		r0, #3
	mov		r0, #4
	mov		r0, #5
	mov		r0, #6
	mov		r0, #7
	mov		r0, #8
	mov		r0, #9
	mov		r0, #1
	mov		r0, #2
	mov		r0, #3
	b	_start
