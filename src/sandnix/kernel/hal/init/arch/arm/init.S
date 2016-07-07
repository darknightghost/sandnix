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

.global		_start
.global		init_stack
.global		gdt

.section	.bss
init_stack:
.rept
.byte
.endr

.section	.data
gdt:
	SEGMENT_DESCRIPTOR	0,				0,					0
descriptor_kernel_data:
	SEGMENT_DESCRIPTOR	0,				0xFFFFF,			DA_DRW | DA_DPL0 | DA_32
descriptor_kernel_code:
	SEGMENT_DESCRIPTOR	0,				0xFFFFF,			DA_CR | DA_DPL0 | DA_32
descriptor_user_data:
	SEGMENT_DESCRIPTOR	0,				0xFFFFF,			DA_DRW | DA_DPL3 | DA_32
descriptor_user_code:
	SEGMENT_DESCRIPTOR	0,				0xFFFFF,			DA_CR | DA_DPL3 | DA_32
descriptor_tss:
	SEGMENT_DESCRIPTOR	0,				0x67,				DA_386TSS | DA_DPL0
gdt_end:

gdtr_value:
	.word		(gdt_end - gdt - 1)
gdt_addr:
	.long		gdt

.section	.text
//EAX == Magic.EBX = Kernel parameters
_start:
	//Check bootloader
	cmpl	$0x36D76289, %eax
	je		_BOOTLOADER_CHECKED
	//if(eax != 0x36D76289) {
		//Reboot
		movb	$0xFE, %al
		outb	%al, $0x64
		hlt
	//}
_BOOTLOADER_CHECKED:
	//Compute offset
	call	_ADDR_1
_ADDR_1:
	popl	%eax
	subl	$_ADDR_1, %eax	//eax = offset

	movl	%ebx, %esi		//esi = kernel parameters

	//Write gdt address
	movl	$gdt, %edx
	subl	%edx, %eax
	movl	$gdt_addr, %ebx
	subl	%ebx, %eax
	movl	%edx, (%ebx)
	
	//Load GDT
	movl	$gdtr_value, %ebx
	subl	%ebx, %eax
	lgdt	(%ebx)

	//Load segments
	pushl	%eax
	movw	$SELECTOR_K_DATA, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	popl	%eax

	//Load stack
	movl	$init_stack, %ebx
	addl	%ebx, $INIT_STACK_SIZE
	subl	%ebx, %eax
	movl	%ebx, %ebp
	movl	%ebx, %esp

	//Push arguments
	pushl	%esi

	//Start paging
	//pushl	$0
	//call	start_paging
	//addl	%esp, $4

	//Jump to kernel mem
	ljmpl	$SELECTOR_K_CODE, $_KERNEL_MEM

_KERNEL_MEM:
	//Load TSS
	movw	%SELECTOR_TSS, %ax
	ltr		%ax

	//Reload stack
	popl	%edi
	movl	$init_stack, %ebx
	addl	%ebx, $INIT_STACK_SIZE
	movl	%ebx, %ebp
	movl	%ebx, %esp
	push	$edi

	//FPU
	fninit
	movl	$0x80010021, %eax
	movl	%eax, %cr0

	//Call c code
	call	kinit
