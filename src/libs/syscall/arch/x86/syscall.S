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

/*
	When a system call happens.
	EAX	:	System call number
	EBX	:	Start address of arguments
	ECX	:	User ESP
	EDX	:	User EIP
*/

.section	.text
.global		do_sys_call

.code32
do_sys_call:
	push	%ebp
	movl	%esp,%ebp
	push	%ebx
	lea		(%ebp+8),%ebx
	movl	%esp,%ecx
	movl	$_RET,%edx
	sysenter
	_RET:
	pop		%ebx
	leave
	ret

