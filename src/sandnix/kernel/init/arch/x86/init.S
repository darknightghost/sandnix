#include "../../init.h"

.section	.text
.global		_start
.global		kernel_header

.code32
.align	4
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

	//Push magic
	pushl	%edx

	//Push boot information
	pushl	%ebx

	//Push offset
	call	_addr2
	_addr2:
	popl	%eax
	subl	$_addr2,%eax
	pushl	%eax

	//Initialize GDT
	//Start paging
	//Jump to kernel memory

	_loop:
	nop
	nop
	nop
	nop
	nop
	nop
	jmp		_loop

.section	.data
