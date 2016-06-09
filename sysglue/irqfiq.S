/*
 * This module contains the assembly shells for IRQ and FIQ, separated
 * from the architectured vectors only by some simple unconditional
 * branch instructions.
 *
 * Note that TI's way of handling interrupts sacrifices Nucleus' ability
 * to nest interrupts and minimize the IRQ-disabled window: if my (Falcon's)
 * understanding is correct, TI's code leaves all further IRQs disabled
 * for the full execution duration of an IRQ handler.  (IRQ handlers are
 * really LISRs, but TI's GSM fw does not use Nucleus' LISR framework.)
 */

	.section	iram.text,"ax",%progbits
	.code	32

	.globl	_INT_IRQ
_INT_IRQ:
        STMDB   sp!,{r0-r4}                 @ used to be a1-a4

/*
 * Thanks to TI for discovering and documenting this apparent ARM7TDMI bug:

BUG correction 1st part -------------------
It looks like there is an issue with ARM7 IRQ masking in the CPSR register
which leads to crashes in Nucleus+ scheduler.
Basically the code below  (correct as LOCKOUT = 0xC0) is used in many places by N+ but do not 
prevent from having an interrupt after the execution of the third line (I mean execution, not 
fetch).
        MRS     a1,CPSR                     ; Pickup current CPSR
        ORR     a1,a1,#LOCKOUT              ; Build interrupt lockout value
        MSR     CPSR,a1                     ; Lockout interrupts
       * IRQ INTERRUPT ! *
       Next instructions...

SW workaround:
When a task is interrupted at this point an interrupted context is stored on its task and will 
be resumed later on at the next instruction but to make a long story short it leads to some 
problem as the OS does not expect to be interrupted there.
Further testing tends to show that the CPSR *seems* to be loaded with the proper masking value 
but that the IRQ is still triggered (has been hardwarewise requested during the instruction 
exectution by the ARM7 core?)
*/

        MRS     a1,spsr                     @ check for the IRQ bug:
        TST     a1,#0x80                    @ if the I - flag is set,
        BNE     IRQBUG                      @ then postpone execution of this IRQ
/* Bug correction 1st part end --------------- */

        SUB     r4,lr,#4                    @ Save IRQ's lr (return address)
        BL      TCT_Interrupt_Context_Save  @ Call context save routine

        BL      IQ_IRQ_isr			@ Call  int. service routine

     /* IRQ interrupt processing is complete.  Restore context- Never 
        returns!  */
        B       TCT_Interrupt_Context_Restore

/* BUG correction 2nd part  ------------------ */
IRQBUG: LDMFD  sp!,{r0-r4}                  @ return from interrupt
        SUBS   pc,r14,#4
/* BUG correction 2nd part end  -------------- */

	.globl	_INT_FIQ
_INT_FIQ:
        STMDB   sp!,{r0-r4}                 @ used to be a1-a4
        SUB     r4,lr,#4                    @ Save FIQ's lr (return address)
        BL      TCT_Interrupt_Context_Save  @ Call context save routine

        BL      IQ_FIQ_isr                  @ Call the FIQ ISR

     /* FIQ interrupt processing is complete.  Restore context- Never 
        returns!  */
        B       TCT_Interrupt_Context_Restore
