/*
 * This is a stripped-down version of the Nucleus PLUS int.s module,
 * reduced to handle just Nucleus PLUS assembly initialization.
 * The IRQ shell code has been moved into irqshell.S, the
 * platform-dependent timer code will be moving somewhere else,
 * and there no ARM architectured vectors in here: that part will be
 * handled by the linked application.
 *
 ************************************************************************
 *                                                                       
 * FILE NAME                              VERSION                        
 *                                                                       
 *      int.s                   Nucleus PLUS\ARM925\Code Composer 1.14.1 
 *                                                                       
 * COMPONENT                                                             
 *                                                                       
 *      IN - Initialization                                              
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This file contains the target processor dependent initialization 
 *      routines and data.                                               
 *                                                                       
 * DATA STRUCTURES                                                       
 *                                                                       
 *      INT_Vectors                         Interrupt vector table       
 *                                                                       
 * FUNCTIONS                                                             
 *                                                                       
 *      INT_Initialize                      Target initialization        
 *      INT_Vectors_Loaded                  Returns a NU_TRUE if all the 
 *                                            default vectors are loaded 
 *      INT_Setup_Vector                    Sets up an actual vector     
 *                                                                       
 * DEPENDENCIES                                                          
 *                                                                       
 *      nucleus.h                           System constants             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      B. Ronquillo     08-28-2002           Released version 1.14.1     
 *                                                                       
 ************************************************************************
 */

#define         NU_SOURCE_FILE

/*
 ******************************
 * INCLUDE ASSEMBLY CONSTANTS *
 ******************************
 * Define constants used in low-level initialization. 
 */

#include "asm_defs.h"

	.code 32

/* stack sizes - matching the original asm_defs.inc for now */
#define	SYSTEM_STACK_SIZE	1024
#define	IRQ_STACK_SIZE		128
#define	FIQ_STACK_SIZE		128
#define	HISR_STACK_SIZE		2048	/* this is for the timer HISR */
#define	HISR_PRIORITY		2	/* ditto */

/*
 **********************************
 * SYSTEM STACK DECLARATIONS      *
 **********************************
 */
    .section "system_stack","aw",%nobits
    .balign  4

INT_System_Stk_Limit:

    .space  SYSTEM_STACK_SIZE

    .balign  4

INT_System_Stack_SP:

/*
 **********************************
 * IRQ STACK DECLARATIONS         *
 **********************************
 */
    .section "irq_stack","aw",%nobits
    .balign  4

    .space  IRQ_STACK_SIZE

    .balign  4

INT_IRQ_Stack_SP:

/*
 **********************************
 * FIQ STACK DECLARATIONS         *
 **********************************
 */
    .section "fiq_stack","aw",%nobits
    .balign  4

    .space  FIQ_STACK_SIZE

    .balign  4

INT_FIQ_Stack_SP:

/*
 **********************************
 * TIMER HISR STACK DECLARATION   *
 **********************************
 */
    .section "timer_hisr_stack","aw",%nobits
    .balign  4

INT_HISR_Stack_Mem:

    .space  HISR_STACK_SIZE

/*
 **********************************
 * LOCAL VARIABLE DECLARATIONS    *
 **********************************
 */
    .text

@ Define various data structure pointers so their addresses can be obtained 
@ in a PC-relative manner. 

HISR_Stack_Ptr:
    .word       TMD_HISR_Stack_Ptr

HISR_Stack_Size:
    .word       TMD_HISR_Stack_Size

HISR_Priority:
    .word       TMD_HISR_Priority

System_Stack:
    .word       TCD_System_Stack

System_Limit:
    .word       TCT_System_Limit

System_Stk_Limit:
    .word       INT_System_Stk_Limit

System_Stack_SP:
    .word       INT_System_Stack_SP

IRQ_Stack_SP:
    .word       INT_IRQ_Stack_SP

FIQ_Stack_SP:
    .word       INT_FIQ_Stack_SP

HISR_Stack_Mem:
    .word       INT_HISR_Stack_Mem

First_Avail_Mem:
    .word       _xram_end		/* ld script will define this */

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      INT_Initialize                                                   
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function sets up the global system stack variable and       
 *      transfers control to the target independent initialization       
 *      function INC_Initialize.  Responsibilities of this function      
 *      include the following:                                           
 *                                                                       
 *             - Setup necessary processor/system control registers      
 *             - Initialize the vector table                             
 *             - Setup the system stack pointers                         
 *             - Setup the timer interrupt                               
 *             - Calculate the timer HISR stack and priority             
 *             - Calculate the first available memory address            
 *             - Transfer control to INC_Initialize to initialize all of 
 *               the system components.                                  
 *                                                                       
 *      Major Revision:                                                  
 *                                                                       
 *          M. Kyle Craig, Accelerated Technology, Inc.                  
 *                                                                       
 *                                                                       
 *                                                                       
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Nothing. This function is the ENTRY point for Nucleus PLUS.      
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      INC_Initialize                      Common initialization        
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      None                                                             
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
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

@VOID    INT_Initialize(void)
@{

    .globl  INT_Initialize
INT_Initialize:

    @ Insure that the processor is in supervisor mode.
    MRS     r0,CPSR                     @ Pickup current CPSR
    BIC     r0,r0,#MODE_MASK            @ Clear the mode bits
    ORR     r0,r0,#SUP_MODE             @ Set the supervisor mode bits
    ORR     r0,r0,#LOCKOUT              @ Insure IRQ/FIQ interrupts are
                                        @ locked out
    MSR     CPSR,r0                     @ Setup the new CPSR

@ Initialize the system stack pointers.  This is done after the BSS is
@ clear because the TCD_System_Stack pointer is a BSS variable!  It is
@ assumed that available memory starts immediately after the end of the
@ BSS section.

    LDR     r10,System_Stk_Limit            @ Pickup the system stack limit (bottom of system stack)
    LDR     r3,System_Limit                 @ Pickup sys stack limit addr
    STR     r10,[r3, #0]                    @ Save stack limit

    LDR     sp,System_Stack_SP              @ Set-up the system stack pointer
    LDR     r3,System_Stack                 @ Pickup system stack address
    STR     sp,[r3, #0]                     @ Save stack pointer

    MRS     r0,CPSR                         @ Pickup current CPSR
    BIC     r0,r0,#MODE_MASK                @ Clear the mode bits
    ORR     r0,r0,#IRQ_MODE                 @ Set the IRQ mode bits
    MSR     CPSR,r0                         @ Move to IRQ mode

    LDR     sp,IRQ_Stack_SP                 @ Setup IRQ stack pointer

    MRS     r0,CPSR                         @ Pickup current CPSR
    BIC     r0,r0,#MODE_MASK                @ Clear the mode bits
    ORR     r0,r0,#FIQ_MODE                 @ Set the FIQ mode bits
    MSR     CPSR,r0                         @ Move to the FIQ mode

    LDR     sp,FIQ_Stack_SP                 @ Setup FIQ stack pointer

@ set up abort and undef mode stacks - code from TI
        MRS     a1,CPSR                     @ Pickup current CPSR
        BIC     a1,a1,#MODE_MASK            @ Clear the mode bits
        ORR     a1,a1,#ABORT_MODE           @ Set the Abort mode bits
        MSR     CPSR,a1                     @ Move to the Abort mode
        LDR     sp,=_Except_Stack_SP	    @ Setup Abort stack pointer

        MRS     a1,CPSR                     @ Pickup current CPSR
        BIC     a1,a1,#MODE_MASK            @ Clear the mode bits
        ORR     a1,a1,#UNDEF_MODE           @ Set the Undefined mode bits
        MSR     CPSR,a1                     @ Move to the Undefined mode
        LDR     sp,=_Except_Stack_SP        @ Setup Undefined stack pointer
                                            @   (should never be used)

@ original Nucleus code continues
    MRS     r0,CPSR                         @ Pickup current CPSR
    BIC     r0,r0,#MODE_MASK                @ Clear mode bits
    ORR     r0,r0,#SUP_MODE                 @ Set the supervisor mode bits
    MSR     CPSR,r0                         @ All interrupt stacks are setup,
                                            @ return to supervisor mode

@ Define the global data structures that need to be initialized by this
@ routine.  These structures are used to define the system timer
@ management HISR.
@    TMD_HISR_Stack_Ptr =        (VOID *) r2;
@    TMD_HISR_Stack_Size =       TIMER_SIZE;
@    TMD_HISR_Priority =         TIMER_PRIORITY;

    LDR     r2,HISR_Stack_Mem               @ Get HISR stack memory address
    LDR     r3,HISR_Stack_Ptr               @ Pickup variable's address
    STR     r2,[r3, #0]                     @ Setup timer HISR stack pointer
    MOV     r1,#HISR_STACK_SIZE             @ Pickup the timer HISR stack size
    LDR     r3,HISR_Stack_Size              @ Pickup variable's address
    STR     r1,[r3, #0]                     @ Setup timer HISR stack size
    MOV     r1,#HISR_PRIORITY               @ Pickup timer HISR priority (0-2)
    LDR     r3,HISR_Priority                @ Pickup variable's address
    STR     r1,[r3, #0]                     @ Setup timer HISR priority

@ Make a call to begin all board specific initialization. 
@ Begin with Initializing the Vector table and replacing
@ default interrupts with Plus IRQs.  Then setup the timer
@ and begin the system clock.

@ FreeCalypso change: we are now using TI's code structure
@ which handles the interrupts and the timers differently.

@   BL      INT_Interrupt_Init              @ Install the vector table
@   BL      INT_Timer_Initialize            @ Initialize the timer 

@ Call INC_Initialize with a pointer to the first available memory
@ address after the compiler's global data.  This memory may be used
@ by the application.
@     INC_Initialize(first_available_memory);

    LDR     r0,First_Avail_Mem              @ Get address of first available memory
      
    B       INC_Initialize                  @ to high-level initialization
@}
