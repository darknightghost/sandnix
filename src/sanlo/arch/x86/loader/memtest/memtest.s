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

.global		memtest
.global		mem_info
.code16

mem_info:
.long		0
.org		20*128+4,0

msg_mem_error:
	.asciz "Memory test Failed!"

memtest:
	pushw	%bp
	movw	%sp,%bp
	pushw	%bx
	pushw	%cx
	pushw	%dx
	pushw	%di
	xorl	%ebx,%ebx
	movw	$(mem_info+4),%di
	_DO_1:
	#do{
		movl	$0xE820,%eax
		movl	$20,%ecx
		movl	$0x534d4150,%edx
		int		$0x15
		jnc		SUCCEED
		pushw	$0x0F
		pushw	$0
		pushw	$2
		pushw	$msg_mem_error
		call	print_at_pos
		addw	$0x08,%sp
		A:
		jmp		A
		SUCCEED:
		addw	$20,%di
		movl	mem_info,%eax
		incl	%eax
		movl	%eax,mem_info
		cmpl	$0,%ebx
		jne		_DO_1
	#}while(ebx!=0)
	popw	%di
	popw	%dx
	popw	%cx
	popw	%bx
	movw	%bp,%sp
	popw	%bp
	ret
