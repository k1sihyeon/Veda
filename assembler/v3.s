	.arch armv8-a
	.file	"v3.c"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
.LFB0:
	.cfi_startproc
	stp	x29, x30, [sp, -32]!
	.cfi_def_cfa_offset 32
	.cfi_offset 29, -32
	.cfi_offset 30, -24
	mov	x29, sp
	str	w0, [sp, 28]
	str	x1, [sp, 16]
.L2:
	ldr	x0, [sp, 16]
	ldr	x0, [x0]
	cmp	x0, 0
	beq	.L3
	ldr	x0, [sp, 16]
	add	x1, x0, 8
	str	x1, [sp, 16]
	ldr	x0, [x0]
	bl	puts
	b	.L2
.L3:
	mov	w0, 0
	ldp	x29, x30, [sp], 32
	.cfi_restore 30
	.cfi_restore 29
	.cfi_def_cfa_offset 0
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Debian 12.2.0-14) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
