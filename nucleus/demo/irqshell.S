/*
 * FreeNucleus Calypso port by Michael Spacefalcon
 *
 * This assembly module contains IRQ handler shell
 * and some closely related support functions.
 *
 * The approach to IRQ handling implemented in the present version
 * is exactly the same as was found in XVilka's original nucleus_plus.tar.gz
 * code targeting OMAP1510.  I personally consider it very dumb and
 * suboptimal, but the present version is merely a proof of concept -
 * changing as little as possible from our starting point will (hopefully)
 * make it easier to get this code to compile, link and maybe even run.
 */

#include "asm_defs.h"
#include "calirq.h"

	.code 32

/*
 **********************************
 * GLOBAL VARIABLE DECLARATIONS   *
 **********************************
 */

	.data

@ Define vector table used by INT_IRQ to branch to necessary ISR

	.globl	INT_IRQ_Vectors
INT_IRQ_Vectors:
    .word     INT_Interrupt_Shell             @ Vector 0
    .word     INT_Interrupt_Shell             @ Vector 1
    .word     INT_Timer_Interrupt             @ Vector 2 - TIMER2
    .word     INT_Interrupt_Shell             @ Vector 3
    .word     INT_Interrupt_Shell             @ Vector 4
    .word     INT_Interrupt_Shell             @ Vector 5
    .word     INT_Interrupt_Shell             @ Vector 6
    .word     INT_Interrupt_Shell             @ Vector 7
    .word     INT_Interrupt_Shell             @ Vector 8
    .word     INT_Interrupt_Shell             @ Vector 9
    .word     INT_Interrupt_Shell             @ Vector 10
    .word     INT_Interrupt_Shell             @ Vector 11
    .word     INT_Interrupt_Shell             @ Vector 12
    .word     INT_Interrupt_Shell             @ Vector 13
    .word     INT_Interrupt_Shell             @ Vector 14
    .word     INT_Interrupt_Shell             @ Vector 15
    .word     INT_Interrupt_Shell             @ Vector 16
    .word     INT_Interrupt_Shell             @ Vector 17
    .word     INT_Interrupt_Shell             @ Vector 18
    .word     INT_Interrupt_Shell             @ Vector 19
    .word     INT_Interrupt_Shell             @ Vector 20

@ Define the order in which the interrupts will be executed by software(INT_IRQ)
@ Level 1 Interrupt Handler
	.globl	INT_IRQ_Priority
INT_IRQ_Priority:
	.word	0
	.word	1
	.word	2
	.word	3
	.word	4
	.word	5
	.word	6
	.word	7
	.word	8
	.word	9
	.word	10
	.word	11
	.word	12
	.word	13
	.word	14
	.word	15
	.word	16
	.word	17
	.word	18
	.word	19
	.word	20
INT_Priority_End:

	.text

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      INT_Setup_Vector                                                 
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function sets up the specified vector with the new vector   
 *      value.  The previous vector value is returned to the caller.     
 *                                                                       
 *                                                                       
 *      Major Revision:                                                  
 *                                                                       
 *          M. Kyle Craig, Accelerated Technology, Inc.                  
 *                                                                       
 *                                                                       
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Application                                                      
 *      TCC_Register_LISR                   Register LISR for vector     
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      vector                              Vector number to setup       
 *      new                                 Pointer to new assembly      
 *                                            language ISR               
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      old vector contents                                              
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        08-27-1994      Created initial version 1.0      
 *      D. Lamie        08-27-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  *INT_Setup_Vector(INT vector, VOID *new)
@{

	.globl  INT_Setup_Vector
INT_Setup_Vector:

@VOID    *old_vector;                        /* Old interrupt vector      */
@VOID   **vector_table;                      /* Pointer to vector table   */

@ Calculate the starting address of the actual vector table.
@    vector_table =  (VOID **) 0;

@ Pickup the old interrupt vector.
@    old_vector =  vector_table[vector];
@    
@ Setup the new interrupt vector.
@    vector_table[vector] =  new;
@    
@ Return the old interrupt vector.
@    return(old_vector);


        LDR     r2, =INT_IRQ_Vectors        @ Load the vector table address
        MOV     r0, r0, LSL #2              @ Multiply vector by 4 to get offset into table
        LDR     r3, [r2,r0]                 @ Load the old pointer
        STR     r1, [r2,r0]                 @ Store the new pointer into the vector table

        MOV     r0, r3                      @ Put the old pointer into the return register

        BX      lr                          @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      INT_Retrieve_Shell                                               
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function retrieves the pointer to the shell interrupt       
 *      service routine.  The shell interrupt service routine calls      
 *      the LISR dispatch routine.                                       
 *                                                                       
 *                                                                       
 *      Major Revision:                                                  
 *                                                                       
 *          M. Kyle Craig, Accelerated Technology, Inc.                  
 *                                                                       
 *                                                                       
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      TCC_Register_LISR                   Register LISR for vector     
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      vector                              Vector number to setup       
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      shell pointer                                                    
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        08-27-1994      Created initial version 1.0      
 *      D. Lamie        08-27-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  *INT_Retrieve_Shell(INT vector)
@{

	.globl  INT_Retrieve_Shell
INT_Retrieve_Shell:

    @ Return the LISR Shell interrupt routine.
    @ return(INT_Vectors[vector]);

        LDR     r1, =INT_IRQ_Vectors        @ Load the vector table address
        MOV     r0, r0, LSL #2              @ Multiply vector by 4 to get offset into table
        LDR     r0, [r1,r0]                 @ Load interrupt handler pointer into return register
                           
        BX      lr                          @ Return to caller

     
@}

/*
 ************************************************************************
 *
 * FUNCTION
 *
 *      _INT_IRQ
 *
 * DESCRIPTION
 *
 *      This routine is the board-specific section for
 *      level 1 interrupt handling
 *
 * CALLED BY                                                             
 *                                                                       
 *      None                                                             
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      TMT_Timer_Interrupt                                              
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      None                                                             
 *                                                                       
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      B. Ronquillo      05-10-00        Created initial version 1.0      
 *                                                                       
 ************************************************************************
 */

	.globl  _INT_IRQ
_INT_IRQ:

    STMDB   sp!,{r0-r4}                     @ Save r0-r4 on temporary IRQ stack
    SUB     lr,lr,#4                        @ Adjust IRQ return address

/*
 ********************************
 * Begin Hardware Specific Code *
 ********************************
 */

    LDR     r3, =INTH_BASE_ADDR             @ load Interrupt Control Base
    @ Get enable register value
    @ original instr was: LDR     r4, [r3,#INT_CNTRL_MIR]
    @ but on the Calypso we have to read two 16-bit regs
    ldrh    r0, [r3, #MASK_IT_REG1]
    ldrh    r1, [r3, #MASK_IT_REG2]
    @ now combine them into a 32-bit word in r4 for the old code to work as-is
    orr     r4, r0, r1, lsl #16

/*
 ******************************
 * End Hardware Specific Code *  
 ******************************
 */

    STMDB   sp!,{r4}                        @ Put the enable register value on the IRQ stack
    MVN     r4,#0                           @ Start with 0xFFFFFFFF to allow nesting of interrupts

/*
 ********************************
 * Begin Hardware Specific Code *
 ********************************
 */

    @ Read Pending reg
    @ original instr was: LDR     r2, [r3,#INT_CNTRL_ITR]
    @ do the same trick as we did for the mask regs
    ldrh    r0, [r3, #IT_REG1]
    ldrh    r1, [r3, #IT_REG2]
    orr     r2, r0, r1, lsl #16

/*
 ******************************
 * End Hardware Specific Code *
 ******************************
 */

    LDR     r3, =INT_IRQ_Priority           @ Get the Priority table address

IRQ_VECTOR_LOOP:
    LDR     r0, [r3,#0]                     @ Load first vector to be checked from priority table
    MOV     r1, #1                          @ Build mask
    MOV     r1, r1, LSL r0                  @ Use vector number to set mask to correct bit position
    TST     r1, r2                          @ Test if pending bit is set
    BNE     IRQ_VECTOR_FOUND                @ If bit is set, branch to found section...

    BIC     r4,r4,r1                        @ Clear mask bit to keep higher priority ints active
    ADD     r3, r3, #4                      @ Move to next word in the priority table
    LDR     r0, =INT_Priority_End           @ Load the end address for the priority table
    CMP     r0, r3                          @ Make sure not at the end of the table (shouldn't happen!)
    BNE     IRQ_VECTOR_LOOP                 @ Continue to loop if not at the end of the table

    @ No bits in pending register set, restore registers and exit interrupt servicing
    ADD     sp,sp,#4                        @ Adjust sp above IRQ enable value
    LDMIA   sp!,{r0-r4}                     @ Restore r0-r4
    STMDB   sp!,{lr}                        @ Put return address for IRQ on stack
    LDMIA   sp!,{pc}^                       @ return to the point of the exception and restore SPSR
 
IRQ_VECTOR_FOUND:

/*
 ********************************
 * Begin Hardware Specific Code *
 ********************************
 */

    LDR     r3, =INTH_BASE_ADDR             @ load Interrupt Control Base

    MVN     r2, r1                          @ Get the inverse of the interrupt vector
    @ Write a zero to the interrupt being handled (IT_REGn)
    cmp     r0, #16
    strloh  r2, [r3, #IT_REG1]
    mov     r2, r2, lsr #16
    strhsh  r2, [r3, #IT_REG2]

    @ Read the Mask reg - just like we did before
    ldrh    r1, [r3, #MASK_IT_REG1]
    ldrh    r2, [r3, #MASK_IT_REG2]
    orr     r2, r1, r2, lsl #16
    @ was LDR     r2, [r3,#INT_CNTRL_MIR]

    ORR     r4, r2, r4                      @ Turn off lower priority pending bits and currently masked bits

    @ write both mask registers
    @ was STR     r4, [r3,#INT_CNTRL_MIR]
    strh    r4, [r3, #MASK_IT_REG1]
    mov     r1, r4, lsr #16
    strh    r1, [r3, #MASK_IT_REG2]

    MOV     r1, #1                          @ Clear the pending interrupt 
    STRH    r1, [r3,#IRQ_CTRL]              @ by writing a 1 to the Control Reg

/*
 ******************************
 * End Hardware Specific Code *
 ******************************
 */

    LDR     r3, =INT_IRQ_Vectors            @ Get IRQ vector table address
    MOV     r2, r0, LSL #2                  @ Multiply vector by 4 to get offset into table
    ADD     r3, r3, r2                      @ Adjust vector table address to correct offset
    LDR     r2, [r3,#0]                     @ Load branch address from vector table

    MOV     PC, r2                          @ Jump to correct branch location based on vector table

@ END: INT_IRQ

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                         
 *     INT_Interrupt_Shell                                               
 *                                                                         
 * DESCRIPTION                                                           
 *                                                                         
 *     Handles all interrupts which use NU_Register_LISR.                
 *                                                                         
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      INT_IRQ                                                          
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      TCT_Dispatch_LISR                                                
 *      TCT_Interrupt_Context_Restore                                    
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      vector (register r0)                                             
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                  
 ************************************************************************
 */

    .globl INT_Interrupt_Shell
INT_Interrupt_Shell:

    MOV     r4,lr                           @ Put IRQ return address into r4

    BL      TCT_Interrupt_Context_Save

    BL      TCC_Dispatch_LISR

    MRS     r1,CPSR                         @ Pickup current CPSR
    BIC     r1,r1,#MODE_MASK                @ Clear the mode bits
    ORR     r1,r1,#(IRQ_MODE_OR_LOCKOUT)    @ Set the IRQ mode bits and Lockout interrupts
    MSR     CPSR,r1                         @ Lockout interrupts/change to IRQ mode

/*
 ********************************
 * Begin Hardware Specific Code *
 ********************************
 */
    LDMIA   sp!,{r1}                        @ Get IRQ enable value off IRQ stack

    LDR     r2, =INTH_BASE_ADDR             @ Get IRQ0 base register address
    @ write it into both mask regs
    @was STR     r1,[r2,#INT_CNTRL_MIR]
    strh    r1, [r2, #MASK_IT_REG1]
    mov     r1, r1, lsr #16
    strh    r1, [r2, #MASK_IT_REG2]
/*
 ******************************
 * End Hardware Specific Code *                                         
 ******************************
 */
    
    MRS     r1,CPSR                         @ Pickup current CPSR
    BIC     r1,r1,#MODE_MASK                @ Clear the mode bits
    ORR     r1,r1,#SUP_MODE                 @ Set the SVC mode bits
    MSR     CPSR,r1                         @ Change to SVC mode

    B       TCT_Interrupt_Context_Restore

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      INT_Timer_Interrupt                                              
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This routine is the board-specific section of the timer          
 *      interrupt handling                                               
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      None                                                             
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      TMT_Timer_Interrupt                                              
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      None                                                             
 *                                                                       
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      B.Ronquillo        05-10-00          Created initial version 1.0   
 *                                                                       
 ************************************************************************
 */

	.globl  INT_Timer_Interrupt
INT_Timer_Interrupt:

/*
 ********************************
 * Begin Hardware Specific Code *
 ********************************
 * Clear the pending timer interrupt
 *
 * This is done in the INT_IRQ function by writing a zero to the   
 * timer bit.
 *
 ******************************
 * End Hardware Specific Code *
 ******************************
 */

    MOV     r4,lr                           @ Put IRQ return address into r4

    BL      TCT_Interrupt_Context_Save

    BL      TMT_Timer_Interrupt             @ Call the timer interrupt
                                            @ processing.

    MRS     r1,CPSR                         @ Pickup current CPSR
    BIC     r1,r1,#MODE_MASK                @ Clear the mode bits
    ORR     r1,r1,#IRQ_MODE                 @ Set the IRQ mode bits
    MSR     CPSR,r1                         @ Change to IRQ mode

/*
 ********************************
 * Begin Hardware Specific Code *
 ********************************
 */
    LDMIA   sp!,{r1}                        @ Get IRQ enable value off IRQ stack

    LDR     r2, =INTH_BASE_ADDR             @ Get IRQ0 base register address
    @ write it into both mask regs
    @was STR     r1,[r2,#INT_CNTRL_MIR]
    strh    r1, [r2, #MASK_IT_REG1]
    mov     r1, r1, lsr #16
    strh    r1, [r2, #MASK_IT_REG2]
/*
 ******************************
 * End Hardware Specific Code *                                         
 ******************************
 */

    MRS     r1,CPSR                         @ Pickup current CPSR
    BIC     r1,r1,#MODE_MASK                @ Clear the mode bits
    ORR     r1,r1,#SUP_MODE                 @ Set the SVC mode bits
    MSR     CPSR,r1                         @ Change to SVC mode

    B       TCT_Interrupt_Context_Restore

@}
@ End of INT_Timer_Interrupt

/*
 * INT_Interrupt_Init function written by Michael Spacefalcon
 * for the FreeNucleus Calypso port - takes the place of
 * INT_Install_Vector_Table, called from INT_Initialize.
 */

	.globl	INT_Interrupt_Init
INT_Interrupt_Init:

@ first mask all interrupts in the INTH

	ldr	r0, =INTH_BASE_ADDR
	mvn	r1, #0
	strh	r1, [r0, #MASK_IT_REG1]
	strh	r1, [r0, #MASK_IT_REG2]

@ now install our IRQ vector into the magic IRAM jump table slot

	ldr	r0, =_INT_IRQ
	mov	r1, #0x800000
	str	r0, [r1, #0x14]

@ we are done!
	bx	lr

/*
 * Need to define an INT_Vectors_Loaded() function
 * that returns 1.
 */
	.globl	INT_Vectors_Loaded
INT_Vectors_Loaded:
	mov	r0, #1
	bx	lr
