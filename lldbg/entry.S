	.section "ext.ram","aw",%nobits
	.balign  4
	.globl	lldbg_stack
lldbg_stack:
	.space	2048
lldbg_init_sp:

	.text
	.code	32
	.globl	lldbg_entry
lldbg_entry:
	stmfd	sp!, {r0-r12,lr}
	mrs	r0, CPSR
	mov	r1, sp
	/* supervisor mode, disable all interrupts */
	msr	CPSR_c, #0xd3
	ldr	sp, =lldbg_init_sp
	/* save entry SP and CPSR */
	ldr	r2, =lldbg_entry_cpsr
	str	r0, [r2]
	ldr	r2, =lldbg_entry_sp
	str	r1, [r2]
	b	lldbg_main
