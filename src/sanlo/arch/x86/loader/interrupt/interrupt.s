	.file	"interrupt.c"
	.text
	.globl	setup_interrupt
	.type	setup_interrupt, @function
setup_interrupt:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$0, %eax
	call	setup_8259A
	movl	$0, %eax
	call	setup_idt
#APP
# 51 "interrupt.c" 1
	sti
	
# 0 "" 2
#NO_APP
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	setup_interrupt, .-setup_interrupt
	.globl	setup_8259A
	.type	setup_8259A, @function
setup_8259A:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$32, %esi
	movl	$17, %edi
	call	out_byte
	movl	$0, %eax
	call	io_delay
	movl	$160, %esi
	movl	$17, %edi
	call	out_byte
	movl	$0, %eax
	call	io_delay
	movl	$33, %esi
	movl	$32, %edi
	call	out_byte
	movl	$0, %eax
	call	io_delay
	movl	$161, %esi
	movl	$40, %edi
	call	out_byte
	movl	$0, %eax
	call	io_delay
	movl	$33, %esi
	movl	$4, %edi
	call	out_byte
	movl	$0, %eax
	call	io_delay
	movl	$161, %esi
	movl	$2, %edi
	call	out_byte
	movl	$0, %eax
	call	io_delay
	movl	$33, %esi
	movl	$1, %edi
	call	out_byte
	movl	$0, %eax
	call	io_delay
	movl	$161, %esi
	movl	$1, %edi
	call	out_byte
	movl	$0, %eax
	call	io_delay
	movl	$33, %esi
	movl	$253, %edi
	call	out_byte
	movl	$0, %eax
	call	io_delay
	movl	$33, %esi
	movl	$255, %edi
	call	out_byte
	movl	$0, %eax
	call	io_delay
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	setup_8259A, .-setup_8259A
	.globl	io_delay
	.type	io_delay, @function
io_delay:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
#APP
# 96 "interrupt.c" 1
	nop
	nop
	nop
	nop
	
# 0 "" 2
#NO_APP
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	io_delay, .-io_delay
	.globl	setup_idt
	.type	setup_idt, @function
setup_idt:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
#APP
# 107 "interrupt.c" 1
	.global bp1
	bp1:
	nop
	nop
	nop
	nop
	
# 0 "" 2
#NO_APP
	movl	$256, %eax
	movl	$int_default+196608, %edx
	movw	%dx, (%rax)
	movl	$256, %eax
	movl	$int_default+196608, %edx
	shrl	$16, %edx
	movw	%dx, 6(%rax)
#APP
# 117 "interrupt.c" 1
	.global bp2
	bp2:
	nop
	nop
	nop
	nop
	
# 0 "" 2
#NO_APP
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	setup_idt, .-setup_idt
	.ident	"GCC: (Ubuntu 4.8.2-19ubuntu1) 4.8.2"
	.section	.note.GNU-stack,"",@progbits
