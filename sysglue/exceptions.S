/*
 * This module contains ARM exception handlers which used to be
 * in chipsetsw/system/Main/int.s in TI's Leonardo code.
 */

    .section "except_stack","aw",%nobits
    .balign  4
    .space   512
    .globl   _Except_Stack_SP
_Except_Stack_SP:

	.text
	.code	32

@ layout of xdump buffer:
@ struct xdump_s {
@     long registers[16] // svc mode registers
@     long cpsr          // svc mode CPSR
@     long exception     // magic word + index of vector taken
@     long stack[20]     // bottom 20 words of usr mode stack
@ }

	.globl	_arm_undefined
_arm_undefined:
	@ store r12 for Xdump_buffer pointer, r11 for index
	stmfd   r13!,{r11,r12}
	mov     r11,#1
	b       save_regs

	.globl	_arm_swi
_arm_swi:
	@ store r12 for Xdump_buffer pointer, r11 for index
	stmfd   r13!,{r11,r12}
	mov     r11,#2
	b       save_regs

	.globl	_arm_abort_prefetch
_arm_abort_prefetch:
	@ store r12 for Xdump_buffer pointer, r11 for index
	stmfd   r13!,{r11,r12}
	mov     r11,#3
	b       save_regs

	.globl	_arm_abort_data
_arm_abort_data:
	@ store r12 for Xdump_buffer pointer, r11 for index
	stmfd   r13!,{r11,r12}
	mov     r11,#4
	b       save_regs

	.globl	_arm_reserved
_arm_reserved:
	ldr	r13,=_Except_Stack_SP	@ mode unknown
	@ store r12 for Xdump_buffer pointer, r11 for index
	stmfd   r13!,{r11,r12}
	mov     r11,#5
	b       save_regs

save_regs:
        ldr     r12,=xdump_buffer
        str     r14,[r12,#4*15] @ save r14_abt (original PC) into r15 slot

        stmia   r12,{r0-r10}    @ save unbanked registers (except r11 and r12)
        ldmfd   r13!,{r0,r1}    @ get original r11 and r12
        str     r0,[r12,#4*11]  @ save original r11
        str     r1,[r12,#4*12]  @ save original r12
        mrs     r0,spsr         @ get original psr
        str     r0,[r12,#4*16]  @ save original cpsr

        mrs     r1,cpsr         @ save mode psr
        bic     r2,r1,#0x1f     @ psr with mode bits cleared
        and     r0,r0,#0x1f     @ get original mode bits
        add     r0,r0,r2

        msr     cpsr,r0		@ move to pre-exception mode
        str     r13,[r12,#4*13] @ save original SP
        str     r14,[r12,#4*14] @ save original LR
        msr     cpsr,r1 	@ restore mode psr

        @ r11 has original index
        orr     r10,r11,#0xDE<<24 @ r10 = 0xDEAD0000 + index of vector taken
        orr     r10,r10,#0xAD<<16
        str     r10,[r12,#4*17] @ save magic + index

        mov     r0,r11          @ put index into 1st argument
        b       dar_exception
