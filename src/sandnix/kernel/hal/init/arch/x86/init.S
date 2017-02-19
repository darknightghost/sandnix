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
#define	MODULE_NAME		hal_init

.global		_start
.global		PRIVATE(arch_init)
.global		gdt
.global		kernel_header
.global		tss_desc_table

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
tss_desc_table:
.rept		MAX_CPU_NUM
	SEGMENT_DESCRIPTOR	0,				0x67,				DA_386TSS | DA_DPL0
.endr
gdt_end:

gdtr_value:
	.word		(gdt_end - gdt - 1)
gdt_addr:
	.long		gdt

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

/* Step 1 : Check bootloader and push kernel parameters. */
#if BOOTLOADER == MULTIBOOT2
	//EAX == Magic.EBX = Kernel parameters
_start:
	cli
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
	//Push kernel parameters
	pushl	%ebx
#endif

/* Step 2 : Prepare running environment for c code. */
	//Compute offset
	call	_ADDR_1
_ADDR_1:
	popl	%eax
	subl	$_ADDR_1, %eax	//eax = offset
	
	//Write gdt address
	movl	$gdt, %edx
	addl	%eax, %edx
	movl	$gdt_addr, %ebx
	addl	%eax, %ebx
	movl	%edx, (%ebx)
	
	//Load GDT
	movl	$gdtr_value, %ebx
	addl	%eax, %ebx
	lgdt	(%ebx)

	//Load segments
	movw	$SELECTOR_K_DATA, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss

	//Start paging
	call	start_paging

	//Jump to kernel mem
	ljmpl	$SELECTOR_K_CODE, $_KERNEL_MEM

_KERNEL_MEM:
	//Reload stack
	popl	%edi
	movl	$init_stack, %ebx
	addl	$DEFAULT_STACK_SIZE, %ebx
	movl	%ebx, %ebp
	movl	%ebx, %esp
	push	%edi

	//Reload GDT
	//Write gdt address
	movl	$gdt, gdt_addr
	
	//Load GDT
	movl	$gdtr_value, %eax
	lgdt	(%eax)

	//FPU
	fninit
	movl	$0x80010021, %eax
	movl	%eax, %cr0

/* Step 3 : Call c code. */
	//Call c code
	call	kinit

PRIVATE(arch_init):
	ret
