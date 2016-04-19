/*
	  Copyright 2016,暗夜幽灵 <darknightghost.cn@gmail.com>

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

#include "arch.h"
#include "../../init.h"
#include "../../../mm/mm.h"

#define	PCI_CONFIG_ADDRESS	0xCF8
#define	PCI_CONFIG_DATA		0xCFC
#define	IOAPIC_INDEX_REG	0xFEC00000
#define	IOAPIC_ID_INDEX		0x00
#define	IOAPIC_DATA_REG		0xFEC00010

.macro	MAKE_PCI_ADDRESS	bus,device,function,offset
	movl	$(0x80000000 | (\bus << 16) | (\device << 11)|(\function << 8) | (\offset & 0xfc)),%eax
.endm

.macro	READ_PCI_DWORD		bus,device,function,offset
	movw	$PCI_CONFIG_ADDRESS,%dx
	MAKE_PCI_ADDRESS	\bus,\device,\function,\offset
	outl	%eax,%dx
	movw	$PCI_CONFIG_DATA,%dx
	inl		%dx,%eax
.endm

.section	.text
.global		_start
.global		kernel_header

.code32
	jmp		_start
.align	8
kernel_header:
//magic
	.long		KERNEL_HEADER_MAGIC
//text_begin
	.long		0
//text_end
	.long		0
//data_begin
	.long		0
//data_end
	.long		0
//checksum
	.long		0 -	KERNEL_HEADER_MAGIC

//GDT
.global gdt
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
descriptor_basic_video:
	SEGMENT_DESCRIPTOR	0xC00B8000,		0xFFFFF,			DA_DRW | DA_DPL0 | DA_32
descriptor_tss:
	SEGMENT_DESCRIPTOR	0,				0x67,				DA_386TSS | DA_DPL0
gdt_end:

gdtr_value:
	.word		(gdt_end - gdt - 1)
gdt_addr:
	.long		gdt

_start:
	//Initialize stack
	movl	%eax,%edx
	movl	$0x7C00,%esp
	call	_addr1
_addr1:
	popl	%eax
	subl	$_addr1,%eax
	addl	$init_stack,%eax
	addl	$KERNEL_STACK_SIZE,%eax
	movl	%eax,%esp

	//Push boot information
	pushl	%ebx

	//Push magic
	pushl	%edx

	//Push offset
	call	_addr2
_addr2:
	popl	%eax
	subl	$_addr2,%eax
	pushl	%eax			//*esp = offset

	//Enable I/O APIC
/*	READ_PCI_DWORD	0, 31, 0, 0xF0
	andl	$0xFFFFC000,%eax
	movl	%eax,%ebx
	movl	%eax,0x31FE(%ebx)
	bts		$8,%eax
	andl	$0xFFFFFF00,%eax
	movl	%eax,0x31FE(%ebx)

	movl	$IOAPIC_ID_INDEX,(IOAPIC_INDEX_REG)
	movl	$0xF000000,(IOAPIC_DATA_REG)*/
	
	//Start paging
	call	start_paging
	movl	%esp,%eax
	subl	(%esp),%eax
	movl	%eax,%esp

	//Load kernel GDT
	movl	$gdt_addr,%ebx
	addl	(%esp),%ebx
	movl	$gdt,%eax
	addl	(%esp),%eax
	movl	%eax,(%ebx)
	movl	$gdtr_value,%eax
	addl	(%esp),%eax
	lgdt	(%eax)

	//Set segment registers
	ljmpl	$SELECTOR_K_CODE,$_kernel_mem_entry
_kernel_mem_entry:
	movw	$SELECTOR_K_DATA,%ax
	movw	%ax,%ds
	movw	%ax,%es
	movw	%ax,%fs
	movw	%ax,%gs
	movw	%ax,%ss
	movw	$SELECTOR_TSS,%ax
	ltr		%ax

	//FPU
	fninit
	movl	$0x80010021,%eax
	movl	%eax,%cr0

	//Call kernel
	addl	$4,%esp
	movl	4(%esp),%eax
	addl	$KERNEL_MEM_BASE,%eax
	movl	%eax,4(%esp)
	//*(esp)	: magic
	//*(esp + 4): p_boot_info
	call	kernel_main

