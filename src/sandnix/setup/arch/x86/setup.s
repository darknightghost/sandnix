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

.equ		KERNEL_STACK_SIZE,(4096*2)

.section	.text
.global		_start

//--------------------------------Macros----------------------------------------
.macro		SEGMENT_DESCRIPTOR base,limit,property
	//Limit1 
	.word		(\limit) & 0xFFFF
	//Base1
	.word		(\base) & 0xFFFF
	//Base2
	.byte		((\base) >> 16) & 0xFF
	//Property1+Limit2+property2,G=1
	.word		(((\limit) >> 8) & 0x0F00) | ((\property) & 0xF0FF) | 0x8000
	//Base3
	.byte		((\base) >> 24) & 0xFF
.endm
//------------------------------Definitions-------------------------------------
//Segment descriptor access

.equ		DA_32,0x4000		#32 bit segment

.equ		DA_DPL0,0x00		#DPL = 0
.equ		DA_DPL1,0x20		#DPL = 1
.equ		DA_DPL2,0x40		#DPL = 2
.equ		DA_DPL3,0x60		#DPL = 3

//Data segment
.equ		DA_DR,0x90			#Read-only
.equ		DA_DRW,0x92			#Readable & writeable
.equ		DA_DRWA,0x93		#Readable writeable & accessed

//Code segment
.equ		DA_C,0x98			#Excute-only
.equ		DA_CR,0x9A			#Excuteable & readable
.equ		DA_CCO,0x9C			#Excute-only conforming code segment
.equ		DA_CCOR,0x9E		#Excuteable & readable conforming code segment

.equ		DA_LDT,0x82			#LDT descriptor
.equ		DA_TaskGate,0x85	#Task gate
.equ		DA_386TSS,0x89		#TSS
.equ		DA_386CGate,0x8C	#Call gate
.equ		DA_386IGate,0x8E	#Interrupt gate
.equ		DA_386TGate,0x8F	#Trap gate

//Segment selectors
.equ		DESCRIPTOR_SIZE,		8
.equ		SELECTOR_K_DATA,		(1 * DESCRIPTOR_SIZE)
.equ		SELECTOR_K_CODE,		(2 * DESCRIPTOR_SIZE)
.equ		SELECTOR_U_DATA,		(3 * DESCRIPTOR_SIZE | 3)
.equ		SELECTOR_U_CODE,		(4 * DESCRIPTOR_SIZE | 3)
.equ		SELECTOR_BASIC_VIDEO,	(5 * DESCRIPTOR_SIZE)
.equ		SELECTOR_TSS,			(6 * DESCRIPTOR_SIZE)
.equ		BASIC_VIDEO_BASE_ADDR,	0xC00B8000

.code32

//---------------------------------GDT------------------------------------------
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
SEGMENT_DESCRIPTOR	0,		0x67,				DA_386TSS | DA_DPL0
gdt_end:

gdtr_value:
	.word		(gdt_end - gdt - 1)
	.long		gdt
//------------------------------Functions---------------------------------------
_start:
		call	start_paging
		movl	$_kernel_mem_entry,%eax
//Jmp to kernel memory
		jmpl	*%eax
_kernel_mem_entry:
//Prepare for new GDT
		lgdt	gdtr_value
//Jmp to kernel_main
		movw	$SELECTOR_K_DATA,%ax
		movw	%ax,%ds
		movw	%ax,%es
		movw	%ax,%ss
		movw	%ax,%fs
		movl	$gdt,%ebx
		addl	$SELECTOR_TSS,%ebx
		movl	$sys_tss,%eax
		movw	%ax,0x02(%ebx)
		shrl	$16,%eax
		movb	%al,0x04(%ebx)
		movb	%ah,0x07(%ebx)
		movw	$SELECTOR_TSS,%ax
		ltr		%ax
		movw	$SELECTOR_BASIC_VIDEO,%ax
		movw	%ax,%gs
		movl	$init_stack,%eax
		addl	$KERNEL_STACK_SIZE,%eax
		movl	%eax,%esp
		movl	%esp,%ebp
		//Initialize fpu environment
		fninit
		movl	%cr0,%eax
		//NE
		bts		$5,%eax
		//EM
		btr		$2,%eax
		//TS
		btr		$3,%eax
		//MP
		btr		$1,%eax
		lcalll	$SELECTOR_K_CODE,$kernel_main
