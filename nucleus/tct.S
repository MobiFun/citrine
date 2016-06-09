/*
 ************************************************************************
 *                                                                       
 *               Copyright Mentor Graphics Corporation 2002              
 *                         All Rights Reserved.                          
 *                                                                       
 * THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS  
 * THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS   
 * SUBJECT TO LICENSE TERMS.                                             
 *                                                                       
 ************************************************************************
 ************************************************************************
 *                                                                       
 * FILE NAME                              VERSION                        
 *                                                                       
 *      tct.s                   Nucleus PLUS\ARM925\Code Composer 1.14.1 
 *                                                                       
 * COMPONENT                                                             
 *                                                                       
 *      TC - Thread Control                                              
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This file contains the target processor dependent routines for   
 *      performing target-dependent scheduling functions.                
 *                                                                       
 * FUNCTIONS                                                             
 *                                                                       
 *      TCT_Control_Interrupts              Enable / disable interrupts  
 *                                          by changing                  
 *                                          TCD_Interrupt_Level          
 *      TCT_Local_Control_Interrupts        Enable/disable interrupts    
 *                                          by not changing              
 *                                          TCD_Interrupt_Level          
 *      TCT_Restore_Interrupts              Restores interrupts to the   
 *                                          level in TCD_Interrupt_Level 
 *      TCT_Build_Task_Stack                Build initial task stack     
 *      TCT_Build_HISR_Stack                Build initial HISR stack     
 *      TCT_Build_Signal_Frame              Build signal handler frame   
 *      TCT_Check_Stack                     Check current stack          
 *      TCT_Schedule                        Schedule the next thread     
 *      TCT_Control_To_Thread               Transfer control to a thread 
 *      TCT_Control_To_System               Transfer control from thread 
 *      TCT_Signal_Exit                     Exit from signal handler     
 *      TCT_Current_Thread                  Returns a pointer to current 
 *                                            thread                     
 *      TCT_Set_Execute_Task                Sets TCD_Execute_Task under  
 *                                            protection from interrupts 
 *      TCT_Protect                         Protect critical section     
 *      TCT_Unprotect                       Unprotect critical section   
 *      TCT_Unprotect_Specific              Release specific protection  
 *      TCT_Set_Current_Protect             Set the thread's current     
 *                                            protection field           
 *      TCT_Protect_Switch                  Switch to protected thread   
 *      TCT_Schedule_Protected              Schedule the protected thread
 *      TCT_Interrupt_Context_Save          Save interrupted context     
 *      TCT_Interrupt_Context_Restore       Restore interrupted context  
 *      TCT_Activate_HISR                   Activate a HISR              
 *      TCT_HISR_Shell                      HISR execution shell         
 *                                                                       
 * DEPENDENCIES                                                          
 *                                                                       
 *      cs_extr.h                           Common Service functions     
 *      tc_extr.h                           Thread Control functions     
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
#include "../include/config.h"

	.code 32

/* TCT_System_Limit is a global variable defined in this module */

	.comm	TCT_System_Limit,4,4

    .text

/*
 **********************************
 * LOCAL VARIABLE DECLARATIONS    *
 **********************************
 * Define pointers to system variables so their addresses may be obtained in a
 * pc-relative manner. 
 */

System_Limit:
        .word   TCT_System_Limit

Int_Level:
        .word   TCD_Interrupt_Level

Task_Shell:
        .word   TCC_Task_Shell

HISR_Shell:
        .word   TCT_HISR_Shell

Signal_Shell:
        .word   TCC_Signal_Shell

Current_Thread:
        .word   TCD_Current_Thread

Execute_HISR:
        .word   TCD_Execute_HISR

Execute_Task:
        .word   TCD_Execute_Task

Time_Slice:
        .word   TMD_Time_Slice

Slice_State:
        .word   TMD_Time_Slice_State

System_Stack:
        .word   TCD_System_Stack

Int_Count:
        .word   TCD_Interrupt_Count

HISR_Tails:
        .word   TCD_Active_HISR_Tails

HISR_Heads:
        .word   TCD_Active_HISR_Heads

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 * TCT_Control_Interrupts                                                
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function enables and disables interrupts as specified by    
 *      the caller.  Interrupts disabled by this call are left disabled  
 *      until the another call is made to enable them.                   
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Application                                                      
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      new_level                           New interrupt enable level   
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      old_level                           Previous interrupt enable    
 *                                            level                      
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *      C. Meredith     03-01-1994      Lockout interrupts while setting 
 *                                        up the new level, resutling in 
 *                                        version 1.1                    
 *      D. Lamie        03-18-1994      Verified version 1.1             
 *      M. Trippi       02-03-1997      Masked the return value to only  
 *                                        return the interrupt bits.     
 *                                        (SPR0252)                      
 *                                                                       
 ************************************************************************
 */

@INT TCT_Control_Interrupts (INT new_level)
@{

        .globl  TCT_Control_Interrupts
TCT_Control_Interrupts:

@INT     old_level;                          Old interrupt level      

@    lock out all interrupts before any checking or changing

@    Obtain the current interrupt lockout posture. 
@    old_level =  TCD_Interrupt_Level;

@    Setup new interrupt lockout posture. 
@    TCD_Interrupt_Level =  new_level;

@    renable interrupts for the specified lockout

@    Return old interrupt lockout level. 
@    return(old_level);

        MRS     r2,CPSR                     @ Pickup current CPSR
        ORR     r2,r2,#LOCKOUT              @ Build lockout CPSR
        MSR     CPSR,r2                     @ Lockout interrupts temporarily
        LDR     r1, Int_Level               @ Pickup interrupt level
        LDR     r3,[r1, #0]                 @ Pickup current interrupt lockout
        BIC     r2,r2,#LOCK_MSK             @ Clear lockout mask
        ORR     r2,r2,r0                    @ Build new CPSR with appropriate
                                            @   interrupts locked out
        STR     r0,[r1,#0]                  @ Save current lockout 
        MSR     CPSR,r2                     @ Setup new CPSR lockout bits
        AND     r0,r3,#LOCK_MSK             @ Return previous lockout (SPR0252)
      
        BX      r14                         @ Return to caller

@}    

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 * TCT_Local_Control_Interrupts                                          
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function enables and disables interrupts as specified by    
 *      the caller.                                                      
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Application                                                      
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      new_level                           New interrupt enable level   
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      old_level                           Previous interrupt enable    
 *                                            level                      
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      C. Meredith     03-01-1994      Created initial version 1.1      
 *      D. Lamie        03-18-1994      Verified version 1.1             
 *      M. Trippi       02-03-1997      Masked the return value to only  
 *                                        return the interrupt bits.     
 *                                        (SPR0252)                      
 *                                                                       
 ************************************************************************
 */

@INT TCT_Local_Control_Interrupts (INT new_level)
@{

	.globl  TCT_Local_Control_Interrupts
TCT_Local_Control_Interrupts:

@INT     old_level;                          Old interrupt level      

@    read in the old level
@    old_level = current interrupt level of processor;

        MRS     r2,CPSR                     @ Pickup current CPSR
        MOV     r3,r2                       @ save the old level

@    clear out the old level and set the new level
@    current interrupt level of processor &= ~LOCKOUT;
@    current interrupt level of processor |= new_level;

        BIC     r2,r2,#LOCK_MSK             @ Clear all current interrupts
        ORR     r2,r2,r0                    @ Build new CPSR with new
                                            @ interrupt level
        MSR     CPSR,r2                     @ Setup new CPSR interrupt bits

@    Return old interrupt lockout level. 
@    return(old_level);

        AND     r0,r3,#LOCK_MSK             @ Return previous lockout (SPR0252)
       
        BX      r14                         @ Return to caller

@}    

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *  TCT_Restore_Interrupts                                               
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function restores interrupts to that specified in the global
 *      TCD_Interrupt_Level variable.                                    
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Application                                                      
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      None.                                                            
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None.                                                            
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      C. Meredith     03-01-1994      Created initial version 1.1      
 *      D. Lamie        03-18-1994      Verified version 1.1             
 *                                                                       
 ************************************************************************
 */

@VOID TCT_Restore_Interrupts  (VOID)
@{

	.globl  TCT_Restore_Interrupts
TCT_Restore_Interrupts:

@ Lock out all interrupts before any checking or changing
@ Obtain the current interrupt lockout posture. 
@ Reload the level base on the TCD_Interrupt_Level variable

        MRS     r1,CPSR                     @ Pickup current CPSR
        MOV     r2,r1                       @ save the CPSR value
        ORR     r1,r1,#LOCKOUT              @ Build lockout CPSR
        MSR     CPSR,r1                     @ Lockout interrupts temporarily
        BIC     r2,r2,#LOCK_MSK             @ Clear current interrupt levels
        LDR     r1,Int_Level                @ Load address of TCD_Interrupt_Level
        LDR     r0,[r1, #0]                 @ Pickup current interrupt lockout
        ORR     r2,r2,r0                    @ Build new CPSR with appropriate
                                            @ interrupts locked out
        MSR     CPSR,r2                     @ Setup new CPSR lockout bits

        BX      r14                         @ Return to caller

@}    

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Build_Task_Stack                                             
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function builds an initial stack frame for a task.  The     
 *      initial stack contains information concerning initial values of  
 *      registers and the task's point of entry.  Furthermore, the       
 *      initial stack frame is in the same form as an interrupt stack    
 *      frame.                                                           
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      TCC_Create_Task                     Create a new task            
 *      TCC_Reset_Task                      Reset the specified task     
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      task                                Task control block pointer   
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Build_Task_Stack(TC_TCB *task)
@{

	.globl  TCT_Build_Task_Stack
TCT_Build_Task_Stack:

    @ Pickup the stack base. 
@    REG_Stack_Base =  (BYTE_PTR) task -> tc_stack_start;

        LDR     r2,[r0,#0x24]               @ Pickup the stack starting address

    @ Pickup the stack size. 
@    REG_Stack_Size =  task -> tc_stack_size;

        LDR     r1,[r0,#0x30]               @ Pickup the stack size in bytes

    @ Calculate the stack ending address. 
@    REG_Stack_End =  REG_Stack_Base + REG_Stack_Size - 1;

        ADD     r3,r1,r2                    @ Compute the beginning of stack
        BIC     r3,r3,#3                    @ Insure word alignment
        SUB     r3,r3,#4                    @ Reserve a word

    @ Save the stack ending address. 
@    task -> tc_stack_end =  REG_Stack_End;

        STR     r3,[r0,#0x28]               @ Save the stack ending address

    @ Reference the task shell. 
@    REG_Function_Ptr =  (VOID *) TCC_Task_Shell;

@ Build an initial stack.  This initial stack frame facilitates an 
@ interrupt return to the TCC_Task_Shell function, which in turn 
@ invokes the application task.  The initial stack frame has the 
@ following format:

@               (Lower Address) Stack Top ->    1       (Interrupt stack type)
@                                               CPSR    Saved CPSR
@                                               r0      Saved r0
@                                               r1      Saved r1
@                                               r2      Saved r2
@                                               r3      Saved r3
@                                               r4      Saved r4
@                                               r5      Saved r5
@                                               r6      Saved r6
@                                               r7      Saved r7
@                                               r8      Saved r8
@                                               r9/sb   Saved r9/sl
@                                               r10/sl   Saved r10/sl
@                                               fp      Saved fp
@                                               r12      Saved r12
@                                               r13      Saved r13
@                                               r14      Saved r14
@               (Higher Address) Stack Bottom-> r15      Saved r15
  

        LDR     r2, Task_Shell              @ Pickup address of shell entry
        STR     r2,[r3], #-4                @ Store entry address on stack
        MOV     r2,#0                       @ Clear value for initial registers
        STR     r2,[r3], #-4                @ Store initial r14
        ADD     r2,r3,#8                    @ Compute initial r13
        STR     r2,[r3], #-4                @ Store initial r13 (Stack Bottom)
        STR     r2,[r3], #-4                @ Store initial r12
        STR     r2,[r3], #-4                @ Store initial fp
        LDR     r2,[r0,#0x24]               @ Pickup the stack starting address
        STR     r2,[r3], #-4                @ Store initial r10/sl
        MOV     r2,#0                       @ Clear value for initial registers
        STR     r2,[r3], #-4                @ Store initial r9/sb
        STR     r2,[r3], #-4                @ Store initial r8
        STR     r2,[r3], #-4                @ Store initial r7
        STR     r2,[r3], #-4                @ Store initial r6
        STR     r2,[r3], #-4                @ Store initial r5
        STR     r2,[r3], #-4                @ Store initial r4
        STR     r2,[r3], #-4                @ Store initial r3
        STR     r2,[r3], #-4                @ Store initial r2
        STR     r2,[r3], #-4                @ Store initial r1
        STR     r2,[r3], #-4                @ Store initial r0
        MSR     CPSR_f,r2                   @ Clear the flags
        MRS     r2,CPSR                     @ Pickup the CPSR
        BIC     r2,r2,#LOCK_MSK             @ Clear initial interrupt lockout
        STR     r2,[r3], #-4                @ Store CPSR on the initial stack
        MOV     r2,#1                       @ Build interrupt stack type (1)
        STR     r2,[r3, #0]                 @ Store stack type on the top 

    @ Save the minimum amount of remaining stack memory. 
@    task -> tc_stack_minimum =  REG_Stack_Size - 72;

        MOV     r2,#72                      @ Size of interrupt stack frame
        SUB     r1,r1,r2                    @ Compute minimum available bytes
        STR     r1,[r0, #0x34]              @ Save in minimum stack area

    @ Save the new stack pointer into the task's control block. 
@    task -> tc_stack_pointer =  (VOID *) Stack_Top;

        STR     r3,[r0, #0x2C]              @ Save stack pointer

        BX      r14                         @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Build_HISR_Stack                                             
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function builds an HISR stack frame that allows quick       
 *      scheduling of the HISR.                                          
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      TCC_Create_HISR                     Create HISR function         
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      hisr                                HISR control block pointer   
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Build_HISR_Stack(TC_HCB *hisr)
@{

	.globl  TCT_Build_HISR_Stack
TCT_Build_HISR_Stack:

    @ Pickup the stack base. 
@    REG_Stack_Base =  (BYTE_PTR) hisr -> tc_stack_start;

        LDR     r2,[r0,#0x24]               @ Pickup the stack starting address

    @ Pickup the stack size. 
@    REG_Stack_Size =  hisr -> tc_stack_size;

        LDR     r1,[r0,#0x30]               @ Pickup the stack size in bytes

    @ Calculate the stack ending address. 
@    REG_Stack_End =  REG_Stack_Base + REG_Stack_Size;

        ADD     r3,r1,r2                    @ Compute the beginning of stack
        BIC     r3,r3,#3                    @ Insure word alignment
        SUB     r3,r3,#4                    @ Reserve a word

    @ Save the stack ending address. 
@    hisr -> tc_stack_end =  REG_Stack_End;

        STR     r3,[r0,#0x28]               @ Save the stack ending address

    @ Reference the HISR shell. 
@    REG_Function_Ptr =  (VOID *) TCT_HISR_Shell;

@ Build an initial stack.  This initial stack frame facilitates an 
@ solicited return to the TCT_HISR_Shell function, which in turn 
@ invokes the appropriate HISR.  The initial HISR stack frame has the 
@ following format:

@               (Lower Address) Stack Top ->    0       (Solicited stack type)
@                !!FOR THUMB ONLY!!             0/0x20  Saved state mask
@                                               r4      Saved r4
@                                               r5      Saved r5
@                                               r6      Saved r6
@                                               r7      Saved r7
@                                               r8      Saved r8
@                                               r9/sb   Saved r9/sl
@                                               r10/sl   Saved r10/sl
@                                               fp      Saved fp
@                                               r12      Saved r12
@               (Higher Address) Stack Bottom-> r15      Saved r15
   

        LDR     r2,HISR_Shell               @ Pickup address of shell entry
        STR     r2,[r3], #-4                @ Store entry address on stack
        ADD     r2,r3,#4                    @ Compute initial r13
        STR     r2,[r3], #-4                @ Store initial r12
        STR     r2,[r3], #-4                @ Store initial fp
        LDR     r2,[r0,#0x24]               @ Pickup the stack starting address
        STR     r2,[r3], #-4                @ Store initial r10/sl
        MOV     r2,#0                       @ Clear value for initial registers
        STR     r2,[r3], #-4                @ Store initial r9/sb
        STR     r2,[r3], #-4                @ Store initial r8
        STR     r2,[r3], #-4                @ Store initial r7
        STR     r2,[r3], #-4                @ Store initial r6
        STR     r2,[r3], #-4                @ Store initial r5
        STR     r2,[r3], #-4                @ Store initial r4
#if 1	/* was .if THUMB */
        STR     r2,[r3], #-4                @ Store initial state mask
#endif
        STR     r2,[r3, #0]                 @ Store solicited stack type on the
                                            @ top of the stack 

    @ Save the minimum amount of remaining stack memory. 
@    hisr -> tc_stack_minimum =  REG_Stack_Size - (ARM)44 or (THUMB)48;

#if 1	/* was .if THUMB */
        MOV     r2,#48                      @ Size of solicited stack frame
#else
        MOV     r2,#44                      @ Size of solicited stack frame
#endif

        SUB     r1,r1,r2                    @ Compute minimum available bytes
        STR     r1,[r0, #0x34]              @ Save in minimum stack area

    @ Save the new stack pointer into the task's control block. 
@    hisr -> tc_stack_pointer =  (VOID *) Stack_Top;

        STR     r3,[r0, #0x2C]              @ Save stack pointer

        BX      r14                         @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Build_Signal_Frame                                           
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function builds a frame on top of the task's stack to       
 *      cause the task's signal handler to execute the next time         
 *      the task is executed.                                            
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      TCC_Send_Signals                    Send signals to a task       
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      task                                Task control block pointer   
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Build_Signal_Frame(TC_TCB *task)
@{

	.globl  TCT_Build_Signal_Frame
TCT_Build_Signal_Frame:

    @ Pickup the stack pointer. 
@    REG_Stack_Ptr =  (BYTE_PTR) task -> tc_stack_pointer;

        LDR     r3,[r0,#0x2c]               @ Pickup the current stack pointer

    @ Reference the Signal shell. 
@    REG_Function_Ptr =  (VOID *) TCC_Signal_Shell;

@ Build a signal stack.  This signal stack frame facilitates an 
@ solicited return to the TCC_Signal_Shell function, which in turn 
@ invokes the appropriate signal handler.  The initial HISR stack frame 
@ has the following format:

@               (Lower Address) Stack Top ->    0       (Solicited stack type)
@                !!FOR THUMB ONLY!!             0/0x20  Saved state mask
@                                               r4      Saved r4
@                                               r5      Saved r5
@                                               r6      Saved r6
@                                               r7      Saved r7
@                                               r8      Saved r8
@                                               r9/sb   Saved r9/sl
@                                               r10/sl   Saved r10/sl
@                                               fp      Saved fp
@                                               r12      Saved r12
@               (Higher Address) Stack Bottom-> r15      Saved r15
   

        LDR     r2,Signal_Shell             @ Pickup address of shell entry
        SUB     r3,r3,#4
        STR     r2,[r3], #-4                @ Store entry address on stack
        ADD     r2,r3,#4                    @ Compute initial r13
        STR     r2,[r3], #-4                @ Store initial r12
        STR     r2,[r3], #-4                @ Store initial fp
        LDR     r2,[r0,#0x24]               @ Pickup the stack starting address
        STR     r2,[r3], #-4                @ Store initial r10/sl
        MOV     r2,#0                       @ Clear value for initial registers
        STR     r2,[r3], #-4                @ Store initial r9/sb
        STR     r2,[r3], #-4                @ Store initial r8
        STR     r2,[r3], #-4                @ Store initial r7
        STR     r2,[r3], #-4                @ Store initial r6
        STR     r2,[r3], #-4                @ Store initial r5
        STR     r2,[r3], #-4                @ Store initial r4
#if 0
        MOV     r1,#0x20                    @ Get initial state mask
        STR     r1,[r3], #-4                @ Store initial state mask  
#else
	STR	r2,[r3], #-4		    @ TCC_Signal_Shell is an ARM proc
#endif

        STR     r2,[r3, #0]                 @ Store solicited stack type on the
                                            @ top of the stack 

    @ Save the new stack pointer into the task's control block. 
@    task -> tc_stack_pointer =  (VOID *) (REG_Stack_Ptr - REG_Stack_Size);

        STR     r3,[r0, #0x2C]              @ Save stack pointer

        BX      r14                         @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Check_Stack                                                  
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function checks the current stack for overflow conditions.  
 *      Additionally, this function keeps track of the minimum amount    
 *      of stack space for the calling thread and returns the current    
 *      available stack space.                                           
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      TCC_Send_Signals                    Send signals to a task       
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      ERC_System_Error                    System error handler         
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      None                                                             
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      available bytes in stack                                         
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@UNSIGNED  TCT_Check_Stack(void)
@{

	.globl  TCT_Check_Stack
TCT_Check_Stack:

@TC_TCB         *thread;
@UNSIGNED       remaining;

    @ Pickup the current task/HISR pointer. 
@    thread =  (TC_TCB *) TCD_Current_Thread;

        LDR     r0,Current_Thread           @ Pickup address of thread pointer
        LDR     r0,[r0,#0]                  @ Pickup thread pointer

    @ Determine if there is a current thread. 
@    if (thread)
@    {

        CMP     r0,#0                       @ Determine if a thread is active
        MOV     r3,#0                       @ Default remaining value
        BEQ     TCT_Skip_Stack_Check        @ If NU_NULL, skip stack checking

        @ Determine if the stack pointers are out of range. 
@        if ((thread -> tc_stack_pointer < thread -> tc_stack_start) ||
@            (thread -> tc_stack_pointer > thread -> tc_stack_end))

        LDR     r2,[r0,#0x24]               @ Pickup start of stack area
        CMP     r13,r2                      @ Compare with current stack ptr
        BLT     TCT_Stack_Range_Error       @ If less, stack is out of range
        LDR     r1,[r0,#0x28]               @ Pickup end of stack area
        CMP     r13,r1                      @ Compare with current stack ptr
        BLE     TCT_Stack_Range_Okay        @ If less, stack range is okay

            @ Stack overflow condition exits. 
@            ERC_System_Error(NU_STACK_OVERFLOW);

TCT_Stack_Range_Error:

        STR     r14,[r13, #4]!              @ Store r14 on the stack
        MOV     r0,#3                       @ Build NU_STACK_OVERFLOW code
        BL      ERC_System_Error            @ Call system error handler.  Note:
                                            @ control is not returned!  
                                            @ Examine stack to find return 
                                            @ address of this routine.

TCT_Stack_Range_Okay:

    @ Calculate the amount of available space on the stack. 
@    remaining =  (BYTE_PTR) thread -> tc_stack_pointer -
@                        (BYTE_PTR) thread -> tc_stack_start;

        SUB     r3,r13,r2                   @ Calculate remaining stack size

    @ Determine if there is enough memory on the stack to save all of the
    @ registers. 
@    if (remaining < 80)

        CMP     r3,#80                      @ Is there enough room for an 
                                            @ interrupt frame?
        BCS     TCT_No_Stack_Error          @ If so, no stack overflow yet

            @ Stack overflow condition is about to happen. 
@            ERC_System_Error(NU_STACK_OVERFLOW);

        STR     r14,[r13, #4]!              @ Store r14 on the stack
        MOV     r0,#3                       @ Build NU_STACK_OVERFLOW code
        BL      ERC_System_Error            @ Call system error handler.  Note:
                                            @ control is not returned!  
                                            @ Examine stack to find return 
                                            @ address of this routine.

TCT_No_Stack_Error:

    @ Determine if this is a new minimum amount of stack space. 
@    if (remaining < thread -> tc_stack_minimum)

        LDR     r2,[r0,#0x34]
        CMP     r3,r2
        STRCC   r3,[r0,#0x34]

        @ Save the new stack minimum. 
@       thread -> tc_stack_minimum =  remaining;
@    }
@    else

        @ Set the remaining bytes to 0. 
@        remaining =  0;

    @ Return the remaining number of bytes on the stack. 
@    return(remaining);

TCT_Skip_Stack_Check:
        MOV     r0,r3                       @ Return remaining bytes

        BX      r14                         @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Schedule                                                     
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function waits for a thread to become ready.  Once a thread 
 *      is ready, this function initiates a transfer of control to that  
 *      thread.                                                          
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      INC_Initialize                      Main initialization routine  
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      TCT_Control_To_Thread               Transfer control to a thread 
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      TCD_Execute_Task                    Pointer to task to execute   
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Schedule(void)
@{

	.globl  TCT_Schedule
TCT_Schedule:

    @ Restore interrupts according to the value contained in 
@    TCD_Interrupt_Level.

        LDR     r1,Int_Level                @ Build address of interrupt level
        MRS     r0,CPSR                     @ Pickup current CPSR
        LDR     r2,[r1, #0]                 @ Pickup current interrupt lockout
        BIC     r0,r0,#LOCK_MSK             @ Clear the interrupt lockout bits
        ORR     r0,r0,r2                    @ Build new interrupt lockout CPSR
        MSR     CPSR,r0                     @ Setup new CPSR 
        LDR     r2,Execute_HISR             @ Pickup TCD_Execute_HISR address
        LDR     r3,Execute_Task             @ Pickup TCD_Execute_Task address

#ifdef INCLUDE_PROVIEW
@ Nucleus ProView Hook
@ We check if upon entering TCT_Schedule we already have a task to excute.
@ if not, we start IDLE.
        LDR     r0,[r2, #0]                 @ Pickup highest priority HISR ptr
        CMP     r0,#0                       @ Is there a HISR active?
        BNE     TCT_Schedule_Thread         @ Found an HISR
        LDR     r0,[r3, #0]                 @ Pickup highest priority Task ptr
        CMP     r0,#0                       @ Is there a task active?
        BNE     TCT_Schedule_Thread         @ If not, start IDLE.
        STR     r2,[r13, #-4]!              @ Save r2 on the stack
        STR     r3,[r13, #-4]!              @ Save r3 on the stack
        BL      _NU_Idle_Hook
        LDR     r3,[r13], #4                @ Recover r2
        LDR     r2,[r13], #4                @ Recover r3
#endif


    @ Wait until a thread (task or HISR) is available to execute.
@    do
@    {
	.globl  TCT_Schedule_Loop
TCT_Schedule_Loop:

@    } while ((!TCD_Execute_HISR) && (!TCD_Execute_Task));

        LDR     r0,[r2, #0]                 @ Pickup highest priority HISR ptr
        CMP     r0,#0                       @ Is there a HISR active?
        BNE     TCT_Schedule_Thread         @ Found an HISR
        LDR     r0,[r3, #0]                 @ Pickup highest priority Task ptr
        CMP     r0,#0                       @ Is there a task active?
#if CONFIG_INCLUDE_L1
	BEQ	_GSM_Small_Sleep
#else
	BEQ	TCT_Schedule_Loop           @ If not, continue the search
#endif

    @ Yes, either a task or an HISR is ready to execute.  Lockout 
    @ interrupts while the thread is transferred to.

TCT_Schedule_Thread:
        MRS     r1,CPSR                     @ Pickup CPSR again
        ORR     r1,r1,#LOCKOUT              @ Build interrupt lockout value
        MSR     CPSR,r1                     @ Lockout interrupts

@ Transfer control to the thread by falling through to the following
@ routine.
@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Control_To_Thread                                            
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function transfers control to the specified thread.  Each   
 *      time control is transferred to a thread, its scheduled counter   
 *      is incremented.  Additionally, time-slicing for task threads is  
 *      enabled in this routine.  The TCD_Current_Thread pointer is      
 *      setup by this function.                                          
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      TCT_Schedule                        Indirectly called            
 *      TCT_Protect                         Protection task switch       
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      thread                              Thread control block pointer 
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Control_To_Thread(TC_TCB *thread)
@{
TCT_Control_To_Thread:

    @ Setup the current thread pointer.
@    TCD_Current_Thread =  (VOID *) thread;

        LDR     r1,Current_Thread           @ Pickup current thread ptr address
        LDR     r2,[r0, #0x1c]              @ Pickup scheduled count
        STR     r0,[r1, #0]                 @ Setup current thread pointer

    @ Increment the thread scheduled counter.
@    thread -> tc_scheduled++;

        LDR     r3,[r0, #0x20]              @ Pickup time slice value
        ADD     r2,r2,#1                    @ Increment the scheduled count
        STR     r2,[r0, #0x1c]              @ Store new scheduled count

    @ Check for time slice option. 
@    if (thread -> tc_cur_time_slice)
@    {
        CMP     r3,#0                       @ Is there a time slice?
        BEQ     TCT_No_Start_TS_1           @ If 0, there is no time slice

        @ Start a time slice.
@        TMD_Time_Slice =        thread -> tc_cur_time_slice;
@        TMD_Time_Slice_State =  0;

        LDR     r2,Time_Slice               @ Pickup address of TMD_Time_Slice
        LDR     r1,Slice_State              @ Pickup address of 
                                            @ TMD_Time_Slice_State
        STR     r3,[r2, #0]                 @ Setup the time slice
        MOV     r2,#0                       @ Build active state flag
        STR     r2,[r1,#0]                  @ Set the active flag
@    }
TCT_No_Start_TS_1:
#ifdef INCLUDE_PROVIEW
@    Nucleus ProView Hook

        STR     r0,[r13, #-4]!              @ Save r0 on the stack
        BL      _NU_Schedule_Task_Hook      @ Branch to RTView
        LDR     r0,[r13], #4                @ Recover return address
#endif


    @ Pickup the stack pointer and resume the thread.
@    REG_Stack_Ptr =  thread -> tc_stack_pointer;

        LDR     r13,[r0, #0x2c]             @ Switch to thread's stack pointer

@ Pop off the saved information associated with the thread. After we
@ determine which type of stack is present.  A 1 on the top of the 
@ stack indicates an interrupt stack, while a 0 on the top of the
@ stack indicates a solicited type of stack.

@ Remember that the interrupt level that is restored must represent
@ the interrupt level in TCD_Interrupt_Level.

        LDR     r1,[r13], #4                @ Pop off the stack type 
        CMP     r1,#1                       @ See if it is an interrupt stack
        BEQ     TCT_Interrupt_Resume        @ If so, an interrupt resume of
                                            @ thread is required
        LDR     r1, Int_Level               @ Pickup address of interrupt 
                                            @ lockout
        MRS     r0,CPSR                     @ Pickup current CPSR
        BIC     r0,r0,#LOCK_MSK             @ Clear lockout mask

        BIC     r0,r0,#0x80000000


        LDR     r2,[r1, #0]                 @ Pickup interrupt lockout mask
        ORR     r0,r0,r2                    @ Build new interrupt lockout mask
#if 1	/* was .if THUMB */
        LDR     r2,[r13], #4                @ Pop off the state mask
        ORR     r0,r0,r2                    @ Set appropriate state 
#endif
        MSR     SPSR,r0                     @ Place it into the SPSR 
        LDMIA   r13!,{r4-r12,r15}^          @ A solicited return is required.  
                                            @ This type of return only 
                                            @ recovers r4-r13 & r15
TCT_Interrupt_Resume:
        LDR     r0,[r13], #4                @ Pop off the CPSR
        LDR     r1,Int_Level                @ Pickup address of interrupt 
                                            @ lockout
        BIC     r0,r0,#LOCK_MSK             @ Clear lockout mask
        LDR     r2,[r1, #0]                 @ Pickup interrupt lockout mask
        ORR     r0,r0,r2                    @ Build new interrupt lockout mask
        MSR     SPSR,r0                     @ Place it into the SPSR 
        LDMIA   r13,{r0-r15}^               @ Recover all registers and resume
                                            @ at point of interrupt
@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Control_To_System                                            
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function returns control from a thread to the system.  Note 
 *      that this service is called in a solicited manner, i.e. it is    
 *      not called from an interrupt thread.  Registers required by the  
 *      compiler to be preserved across function boundaries are saved by 
 *      this routine.  Note that this is usually a sub-set of the total  
 *      number of available registers.                                   
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Other Components                                                 
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      TCT_Schedule                        Schedule the next thread     
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
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *      C. Meredith     03-01-1994      Corrected problem in time-slice  
 *                                        reset logic, resulting in      
 *                                        version 1.1                    
 *      D. Lamie        03-18-1994      Verified version 1.1             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Control_To_System(void)
@{

	.globl  TCT_Control_To_System
TCT_Control_To_System:

    @ Lockout interrupts. 

        MRS     r0,CPSR                     @ Pickup current CPSR
        ORR     r0,r0,#LOCKOUT              @ Build interrupt lockout value
        MSR     CPSR,r0                     @ Lockout interrupts

    @ Save a minimal context of the thread. 

        STMDB   r13!,{r4-r12,r14}           @ Save minimal context of thread on
                                            @ the current stack
#if 1	/* was .if THUMB */
        MOV     r2,r14                      @ Determine what state the caller
        MOV     r2,r2,LSL #31               @ was in and build an
        MOV     r2,r2,LSR #26               @ appropriate state mask
        STR     r2,[r13, #-4]!              @ Place it on the stack
#endif

        MOV     r2,#0                       @ Build solicited stack type value
                                            @ and NU_NULL value
        STR     r2,[r13, #-4]!              @ Place it on the top of the stack

    @ Setup a pointer to the thread control block. 
@    REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;

        LDR     r1,Current_Thread           @ Pickup current thread ptr address
        LDR     r0,[r1, #0]                 @ Pickup current thread pointer

    @ Clear the current thread control block pointer. 
@    TCD_Current_Thread =  NU_NULL;

        LDR     r3,Slice_State              @ Pickup time slice state address
        STR     r2,[r1, #0]                 @ Set current thread pointer to 
                                            @ NU_NULL

@ Check to see if a time slice is active.  If so, copy the original time
@ slice into the current time slice field of the task's control block.
@    if (TMD_Time_Slice_State == 0)
@    {
        LDR     r1,[r3, #0]                 @ Pickup time slice state flag
        CMP     r1,#0                       @ Compare with active value
        BNE     TCT_No_Stop_TS_1            @ If non-active, don't disable

    
    @ Insure that the next time the task runs it gets a fresh time 
    @ slice. 
@       REG_Thread_Ptr -> tc_cur_time_slice =  REG_Thread_Ptr -> tc_time_slice;

        LDR     r1,[r0, #0x40]              @ Pickup original time slice

        @ Clear any active time slice by setting the state to NOT_ACTIVE. 
@        TMD_Time_Slice_State =  1;

        MOV     r2,#1                       @ Build disable value
        STR     r2,[r3, #0]                 @ Disable time slice
        STR     r1,[r0, #0x20]              @ Reset current time slice
@    }
TCT_No_Stop_TS_1:

     @ Save off the current stack pointer in the control block. 
@    REG_Thread_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr;

        STR     r13,[r0, #0x2c]             @ Save the thread's stack pointer

     @ Clear the task's current protection. 
@    (REG_Thread_Ptr -> tc_current_protect) -> tc_tcb_pointer =  NU_NULL;
@    REG_Thread_Ptr -> tc_current_protect =  NU_NULL;

        LDR     r1,[r0, #0x38]              @ Pickup current thread pointer
        MOV     r2,#0                       @ Build NU_NULL value
        STR     r2,[r0, #0x38]              @ Clear the protect pointer field
        STR     r2,[r1, #0]                 @ Release the actual protection

    @ Switch to the system stack. 
@    REG_Stack_Ptr =  TCD_System_Stack;

        LDR     r1, System_Stack            @ Pickup address of stack pointer
        LDR     r2, System_Limit            @ Pickup address of stack limit ptr
        LDR     r13,[r1, #0]                @ Switch to system stack
        LDR     r10,[r2, #0]                @ Setup system stack limit

    @ Finished, return to the scheduling loop. 

        B       TCT_Schedule                @ Return to scheduling loop
@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Signal_Exit                                                  
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function exits from a signal handler.  The primary purpose  
 *      of this function is to clear the scheduler protection and switch 
 *      the stack pointer back to the normal task's stack pointer.       
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      TCC_Signal_Shell                    Signal handling shell func   
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      TCT_Schedule                        Scheduler                    
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
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *      C. Meredith     03-01-1994      Corrected problem in time-slice  
 *                                        reset logic, resulting in      
 *                                        version 1.1                    
 *      D. Lamie        03-18-1994      Verified version 1.1             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Signal_Exit(void)
@{

	.globl  TCT_Signal_Exit
TCT_Signal_Exit:

    @ Lockout interrupts. 

        MRS     r3,CPSR                     @ Pickup current CPSR
        ORR     r3,r3,#LOCKOUT              @ Build lockout value
        MSR     CPSR,r3                     @ Lockout interrupts

    @ Setup a pointer to the thread control block. 
@    REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;

        LDR     r1,Current_Thread           @ Pickup address of thread pointer
        MOV     r2,#0                       @ Build NU_NULL value
        LDR     r0,[r1,#0]                  @ Pickup current thread pointer

    @ Clear the current thread control block. 
@    TCD_Current_Thread =  NU_NULL;

        LDR     r3,Slice_State              @ Pickup time slice state address
        STR     r2,[r1, #0]                 @ Clear current thread pointer

    @ Check to see if a time slice is active.  If so, copy the original time
    @ slice into the current time slice field of the task's control block.
@    if (TMD_Time_Slice_State == 0)
@    {

        LDR     r1,[r3, #0]                 @ Pickup time slice state flag
        CMP     r1,#0                       @ Compare with active value
        BNE     TCT_No_Stop_TS_2            @ If non-active, don't disable

        @ Insure that the next time the task runs it gets a fresh time 
        @ slice. 
@       REG_Thread_Ptr -> tc_cur_time_slice =  REG_Thread_Ptr -> tc_time_slice;

        LDR     r1,[r0, #0x40]              @ Pickup original time slice

        @ Clear any active time slice by setting the state to NOT_ACTIVE. 
@        TMD_Time_Slice_State =  1;

        MOV     r2,#1                       @ Build disable value
        STR     r2,[r3, #0]                 @ Disable time slice
        STR     r1,[r0, #0x20]              @ Reset current time slice
@    }
TCT_No_Stop_TS_2:

    @ Switch back to the saved stack.  The saved stack pointer was saved
    @ before the signal frame was built. 
@    REG_Thread_Ptr -> tc_stack_pointer =  
@                                REG_Thread_Ptr -> tc_saved_stack_ptr;

        LDR     r1,[r0, #0x3c]              @ Pickup saved stack pointer
        STR     r1,[r0, #0x2c]              @ Place in current stack pointer

    @ Clear the task's current protection. 
@    (REG_Thread_Ptr -> tc_current_protect) -> tc_tcb_pointer =  NU_NULL;
@    REG_Thread_Ptr -> tc_current_protect =  NU_NULL;

        LDR     r1,[r0, #0x38]              @ Pickup current thread pointer
        MOV     r2,#0                       @ Build NU_NULL value
        STR     r2,[r0, #0x38]              @ Clear the protect pointer field
        STR     r2,[r1, #0]                 @ Release the actual protection

    @ Switch to the system stack. 
@    REG_Stack_Ptr =  (BYTE_PTR) TCD_System_Stack;

        LDR     r1, System_Stack            @ Pickup address of stack pointer
        LDR     r2, System_Limit            @ Pickup address of stack limit ptr
        LDR     r13,[r1, #0]                @ Switch to system stack
        LDR     r10,[r2, #0]                @ Setup system stack limit

    @ Finished, return to the scheduling loop. 

        B       TCT_Schedule                @ Return to scheduling loop
@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Current_Thread                                               
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function returns the current thread pointer.                
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Application                                                      
 *      System Components                                                
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      None                                                             
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      Pointer to current thread                                        
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  *TCT_Current_Thread(void)
@{

	.globl  TCT_Current_Thread
TCT_Current_Thread:

    @ Return the current thread pointer. 
@    return(TCD_Current_Thread);

        LDR     r0, Current_Thread          @ Pickup address of thread pointer
        LDR     r0,[r0, #0]                 @ Pickup current thread pointer

        BX      r14                         @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Set_Execute_Task                                             
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function sets the current task to execute variable under    
 *      protection against interrupts.                                   
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      TCC Scheduling Routines                                          
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      task                                Pointer to task control block
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      TCD_Execute_Task                    Modified variable            
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Set_Execute_Task(TC_TCB *task)
@{

	.globl  TCT_Set_Execute_Task
TCT_Set_Execute_Task:

    @ Now setup the TCD_Execute_Task pointer. 
@    TCD_Execute_Task =  task;

        LDR     r1, Execute_Task            @ Pickup execute task ptr address
        STR     r0,[r1,#0]                  @ Setup new task to execute

        BX      r14                         @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Protect                                                      
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function protects against multiple thread access.           
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Application                                                      
 *      System Components                                                
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      protect                            Pointer to protection block   
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Protect(TC_PROTECT *protect)
@{

	.globl  TCT_Protect
TCT_Protect:

    @ Determine if the caller is in a task or HISR thread. 
@    if (TCD_Current_Thread)
@    {

        LDR     r1,Current_Thread           @ Pickup current thread ptr address
        LDR     r3,[r1, #0]                 @ Pickup current thread pointer
        CMP     r3,#0                       @ Check to see if it is non-NULL
        BEQ     TCT_Skip_Protect            @ If NULL, skip protection

        @ Lockout interrupts. 

        MRS     r1,CPSR                     @ Pickup current CPSR
        ORR     r1,r1,#LOCKOUT              @ Place lockout value in
        MSR     CPSR,r1                     @ Lockout interrupts

        @ Wait until the protect structure is available. 
@        while (protect -> tc_tcb_pointer != NU_NULL)
@        {

TCT_Protect_Loop:
        LDR     r1,[r0, #0]                 @ Pickup protection owner field
        CMP     r1,#0                       @ Is there any protection?
        BEQ     TCT_Protect_Available       @ If NU_NULL, no current protection

        @ Protection structure is not available. 

        @ Indicate that another thread is waiting. 
@         protect -> tc_thread_waiting =  1;

        MOV     r2,#1                       @ Build thread waiting flag
        STR     r2,[r0, #4]                 @ Set waiting field

        @ Directly schedule the thread waiting. 
@       TCT_Schedule_Protected(protect -> tc_tcb_pointer);

        STR     r0,[r13, #-4]!              @ Save r0 on the stack
        STR     r14,[r13, #-4]!             @ Save r14 on the stack
        MOV     r0,r3                       @ Place current thread into r0
        BL      TCT_Schedule_Protected      @ Call routine to schedule the 
                                            @ owner of the thread

        LDR     r14,[r13], #4               @ Recover saved r14
        LDR     r0,[r13], #4                @ Recover saved r0

        @ Lockout interrupts. 

        LDR     r1,Current_Thread           @ Pickup current thread ptr address
        LDR     r3,[r1, #0]                 @ Pickup current thread pointer
        MRS     r1,CPSR                     @ Pickup current CPSR
        ORR     r1,r1,#LOCKOUT              @ Place lockout value in
        MSR     CPSR,r1                     @ Lockout interrupts
        B       TCT_Protect_Loop            @ Examine protect flags again
@        }
TCT_Protect_Available:

        @ Protection structure is available. 
    
        @ Indicate that this thread owns the protection. 
@        protect -> tc_tcb_pointer =  TCD_Current_Thread;

        STR     r3,[r0, #0]                 @ Indicate calling thread owns this
                                            @ protection

        @ Clear the thread waiting flag. 
@        protect -> tc_thread_waiting =  0;

        MOV     r2,#0                       @ Clear value
        STR     r2,[r0, #4]                 @ Clear the thread waiting flag

@ Save the protection pointer in the thread's control block.  Note 
@ that both task and HISR threads share the same control block 
@ format. 
@        REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;
@        REG_Thread_Ptr -> tc_current_protect =  protect;

        STR     r0,[r3, #0x38]              @ Setup current protection

        @ Restore interrupts. 

        LDR     r2,Int_Level                @ Pickup address of interrupt level
        MRS     r1,CPSR                     @ Pickup current CPSR
        LDR     r3,[r2, #0]                 @ Pickup interrupt lockout level
        BIC     r1,r1,#LOCK_MSK             @ Clear lockout bits
        ORR     r1,r1,r3                    @ Build new interrupt lockout
        MSR     CPSR,r1                     @ Setup CPSR appropriately
@    }

TCT_Skip_Protect:

        BX      r14                         @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Unprotect                                                    
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function releases protection of the currently active        
 *      thread.  If the caller is not an active thread, then this        
 *      request is ignored.                                              
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Application                                                      
 *      System Components                                                
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
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
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Unprotect(void)
@{

	.globl  TCT_Unprotect
TCT_Unprotect:


    @ Determine if the caller is in a task or HISR thread. 
@    if (TCD_Current_Thread)
@    {

        LDR     r1,Current_Thread           @ Pickup current thread ptr address
        LDR     r3,[r1, #0]                 @ Pickup current thread pointer
        CMP     r3,#0                       @ Check to see if it is non-NULL
        BEQ     TCT_Skip_Unprotect          @ If NULL, skip unprotection

        @ Setup a thread control block pointer. 
@        REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;

        @ Determine if there is a currently active protection. 
@        if (REG_Thread_Ptr -> tc_current_protect)
@        {

        LDR     r0,[r3, #0x38]              @ Pickup current protect field
        CMP     r0,#0                       @ Is there a protection in force?
        BEQ     TCT_Skip_Unprotect          @ If not, nothing is protected

        @ Lockout interrupts. 

        MRS     r1,CPSR                     @ Pickup current CPSR
        ORR     r1,r1,#LOCKOUT              @ Place lockout value in
        MSR     CPSR,r1                     @ Lockout interrupts

        @ Yes, this thread still has this protection structure. 
@            REG_Protect_Ptr =  REG_Thread_Ptr -> tc_current_protect;

        @ Is there a higher priority thread waiting for the protection
        @ structure? 
@            if (REG_Protect_Ptr -> tc_thread_waiting)

        LDR     r2,[r0, #4]                 @ Pickup thread waiting flag
        CMP     r2,#0                       @ Are there any threads waiting?
        BEQ     TCT_Not_Waiting_Unpr        @ If not, just release protection

@                Transfer control to the system.  Note that this 
@                   automatically clears the current protection and it returns
@                   to the caller of this routine instead of this routine. 
@                TCT_Control_To_System();

        B       TCT_Control_To_System       @ Return control to the system

@            else
@            {
TCT_Not_Waiting_Unpr:

        @ Clear the protection. 
@                REG_Thread_Ptr -> tc_current_protect =  NU_NULL;
@                REG_Protect_Ptr -> tc_tcb_pointer =  NU_NULL;

        MOV     r2,#0                       @ Build NU_NULL value
        STR     r2,[r0, #0]                 @ Release the protection
        STR     r2,[r3, #0x38]              @ Clear protection pointer in the
                                            @ control block

@            }

TCT_Not_Protected:
        @ Restore interrupts again. 

        LDR     r2,Int_Level                @ Pickup address of interrupt level
        MRS     r1,CPSR                     @ Pickup current CPSR
        LDR     r3,[r2, #0]                 @ Pickup interrupt lockout level
        BIC     r1,r1,#LOCK_MSK             @ Clear lockout bits
        ORR     r1,r1,r3                    @ Build new interrupt lockout
        MSR     CPSR,r1                     @ Setup CPSR appropriately

@        }
@    }
TCT_Skip_Unprotect:

        BX      r14                         @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Unprotect_Specific                                           
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function releases a specific protection structure.          
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Application                                                      
 *      System Components                                                
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      protect                            Pointer to protection block   
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *      C. Meredith     03-01-1994      Corrected problem in time-slice  
 *                                        reset logic, corrected bug     
 *                                        using protect ptr, resulting   
 *                                        in version 1.1                 
 *      D. Lamie        03-18-1994      Verified version 1.1             
 *                                                                       
 ************************************************************************
 */

@VOID  Specific(TC_PROTECT *protect)
@{

	.globl  TCT_Unprotect_Specific
TCT_Unprotect_Specific:

    @ Determine if the caller is in a task or HISR thread. 
@    if (TCD_Current_Thread)
@    {

        LDR     r1,Current_Thread           @ Pickup current thread ptr address
        LDR     r3,[r1, #0]                 @ Pickup current thread pointer
        CMP     r3,#0                       @ Check to see if it is non-NULL
        BEQ     TCT_Skip_Unprot_Spec        @ If NULL, skip unprotect specific

        @ Lockout interrupts. 

        MRS     r1,CPSR                     @ Pickup current CPSR
        ORR     r1,r1,#LOCKOUT              @ Place lockout value in
        MSR     CPSR,r1                     @ Lockout interrupts

        @ Clear the protection pointer. 
@        protect -> tc_tcb_pointer =  NU_NULL;

        MOV     r2,#0                       @ Build NU_NULL value
        STR     r2,[r0, #0]                 @ Clear protection ownership

        @ Determine if a thread is waiting. 
@        if (protect -> tc_thread_waiting)
@        {

        LDR     r1,[r0, #4]                 @ Pickup the waiting field
        CMP     r1,#0                       @ Is there another thread waiting?
        BEQ     TCT_Not_Waiting_Unspec      @ No, restore interrupts and return

        @ A higher-priority thread is waiting. 

        @ Save a minimal context of the thread. 

        STMDB   r13!,{r4-r12,r14}           @ Save minimal context of thread on
                                            @ the current stack

#if 1	/* was .if THUMB */
        MOV     r2,r14                      @ Determine what state the caller
        MOV     r2,r2,LSL #31               @ was in and build an
        MOV     r2,r2,LSR #26               @ appropriate state mask
        STR     r2,[r13, #-4]!              @ Place it on the stack
#endif

        MOV     r2,#0                       @ Build solicited stack type value
                                            @ and NU_NULL value
        STR     r2,[r13, #-4]!              @ Place it on the top of the stack

        @ Setup a pointer to the thread control block. 
@            REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;

        LDR     r1,Current_Thread           @ Pickup current thread ptr address
        LDR     r0,[r1, #0]                 @ Pickup current thread pointer

        @ Clear the current thread control block pointer. 
@            TCD_Current_Thread =  NU_NULL;

        LDR     r3,Slice_State              @ Pickup time slice state address
        STR     r2,[r1, #0]                 @ Set current thread pointer to 
                                            @ NU_NULL

        @ Check to see if a time slice is active.  If so, copy the 
        @ original time slice into the current time slice field of the 
        @ thread's control block.
@            if (TMD_Time_Slice_State == 0)
@            {

        LDR     r1,[r3, #0]                 @ Pickup time slice state flag
        CMP     r1,#0                       @ Compare with active value
        BNE     TCT_No_Stop_TS_3            @ If non-active, don't disable

        @ Insure that the next time the task runs it gets a fresh time
        @ slice. 
@                REG_Thread_Ptr -> tc_cur_time_slice =  
@                                        REG_Thread_Ptr -> tc_time_slice;

        LDR     r1,[r0, #0x40]              @ Pickup original time slice

        @ Clear any active time slice by setting the state to 
        @ NOT_ACTIVE. 
@                TMD_Time_Slice_State =  1;

        MOV     r2,#1                       @ Build disable value
        STR     r2,[r3, #0]                 @ Disable time slice
        STR     r1,[r0, #0x20]              @ Reset current time slice
@            }

TCT_No_Stop_TS_3:

        @ Save off the current stack pointer in the control block. 
@            REG_Thread_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr;

        STR     r13,[r0, #0x2c]             @ Save the thread's stack pointer

        @ Switch to the system stack. 
@            REG_Stack_Ptr =  TCD_System_Stack;

        LDR     r1,System_Stack             @ Pickup address of stack pointer
        LDR     r2,System_Limit             @ Pickup address of stack limit ptr
        LDR     r13,[r1, #0]                @ Switch to system stack
        LDR     r10,[r2, #0]                @ Setup system stack limit

        @ Finished, return to the scheduling loop. 

        B       TCT_Schedule                @ Return to scheduling loop

@        }
@        else
@        {
TCT_Not_Waiting_Unspec:

        @ No higher-priority thread is waiting. 

@            Restore interrupts. 

        LDR     r2,Int_Level                @ Pickup address of interrupt level
        MRS     r1,CPSR                     @ Pickup current CPSR
        LDR     r3,[r2, #0]                 @ Pickup interrupt lockout level
        BIC     r1,r1,#LOCK_MSK             @ Clear lockout bits
        ORR     r1,r1,r3                    @ Build new interrupt lockout
        MSR     CPSR,r1                     @ Setup CPSR appropriately

@        }
@    }

TCT_Skip_Unprot_Spec:

        BX      r14                         @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Set_Current_Protect                                          
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function sets the current protection field of the current   
 *      thread's control block to the specified protection pointer.      
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      TCC_Resume_Task                    Resume task function          
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      protect                            Pointer to protection block   
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Set_Current_Protect(TC_PROTECT *protect)
@{

	.globl  TCT_Set_Current_Protect
TCT_Set_Current_Protect:

    @ Determine if the caller is in a task or HISR thread. 
@    if (TCD_Current_Thread)
@    {

        LDR     r1,Current_Thread           @ Pickup current thread ptr address
        LDR     r3,[r1, #0]                 @ Pickup current thread pointer
        CMP     r3,#0                       @ Check to see if a thread is 
                                            @ active

        @ Point at the current thread control block. 
@        REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;

        @ Modify the current protection. 
@        REG_Thread_Ptr -> tc_current_protect =  protect;

        STRNE   r0,[r3, #0x38]              @ Setup new protection
@    }


        BX      r14                         @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Protect_Switch                                               
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function waits until a specific task no longer has any      
 *      protection associated with it.  This is necessary since task's   
 *      cannot be suspended or terminated unless they have released all  
 *      of their protection.                                             
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      System Components                                                
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      thread                             Pointer to thread control blk 
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Protect_Switch(VOID *thread)
@{

	.globl  TCT_Protect_Switch
TCT_Protect_Switch:

        @ Lockout interrupts. 

        MRS     r1,CPSR                     @ Pickup current CPSR
        ORR     r1,r1,#LOCKOUT              @ Place lockout value in
        MSR     CPSR,r1                     @ Lockout interrupts

@    REG_Thread_Ptr =  (TC_TCB *) thread;

    @ Wait until the specified task has no protection associated with it. 
@    while (REG_Thread_Ptr -> tc_current_protect)
@    {

        LDR     r1,[r0, #0x38]              @ Pickup protection of specified
                                            @ thread
        CMP     r1,#0                       @ Does the specified thread have
                                            @ an active protection?
        BEQ     TCT_Switch_Done             @ If not, protect switch is done

        @ Let the task run again in an attempt to clear its protection. 

        @ Indicate that a higher priority thread is waiting. 
@        (REG_Thread_Ptr -> tc_current_protect) -> tc_thread_waiting =  1;

        MOV     r2,#1                       @ Build waiting flag value
        STR     r2,[r1, #4]                 @ Set waiting flag of the 
                                            @ protection owned by the other
                                            @ thread

        @ Directly schedule the thread waiting. 
@        TCT_Schedule_Protected((REG_Thread_Ptr -> tc_current_protect)  
@                                                        -> tc_tcb_pointer);

        LDR     r2,Current_Thread           @ Pickup current thread ptr address
        STR     r0,[r13, #-4]!              @ Save r0 on the stack
        STR     r14,[r13, #-4]!             @ Save r14 on the stack
        MOV     r1,r0                       @ Move new thread into r1
        LDR     r0,[r2, #0]                 @ Pickup current thread pointer
        BL      TCT_Schedule_Protected      @ Call routine to schedule the 
                                            @ owner of the thread

        LDR     r14,[r13], #4               @ Recover saved r14
        LDR     r0,[r13], #4                @ Recover saved r0

        @ Lockout interrupts. 

        B       TCT_Protect_Switch          @ Branch to top of routine and 
                                            @ start over
@    }
TCT_Switch_Done:

        @ Restore interrupts. 

        LDR     r2,Int_Level                @ Pickup address of interrupt level
        MRS     r1,CPSR                     @ Pickup current CPSR
        LDR     r3,[r2, #0]                 @ Pickup interrupt lockout level
        BIC     r1,r1,#LOCK_MSK             @ Clear lockout bits
        ORR     r1,r1,r3                    @ Build new interrupt lockout
        MSR     CPSR,r1                     @ Setup CPSR appropriately


        BX      r14                         @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Schedule_Protected                                           
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function saves the minimal context of the thread and then   
 *      directly schedules the thread that has protection over the       
 *      the thread that called this routine.                             
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      TCT_Protect                                                      
 *      TCT_Protect_Switch                                               
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      TCT_Control_To_Thread               Transfer control to protected
 *                                              thread                   
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
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *      C. Meredith     03-01-1994      Corrected problem in time-slice  
 *                                        reset logic, resulting in      
 *                                        version 1.1                    
 *      D. Lamie        03-18-1994      Verified version 1.1             
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Schedule_Protected(VOID *thread)
@{

	.globl	TCT_Schedule_Protected
TCT_Schedule_Protected:

    @ Interrupts are already locked out by the caller. 

    @ Save minimal context required by the system. 

        STMDB   r13!,{r4-r12,r14}           @ Save minimal context of thread on
                                            @ the current stack

#if 1	/* was .if THUMB */
        MOV     r2,r14                      @ Determine what state the caller
        MOV     r2,r2,LSL #31               @ was in and build an
        MOV     r2,r2,LSR #26               @ appropriate state mask
        STR     r2,[r13, #-4]!              @ Place it on the stack
#endif

        MOV     r2,#0                       @ Build solicited stack type value
                                            @ and NU_NULL value
        STR     r2,[r13, #-4]!              @ Place it on the top of the stack
        MOV     r4,r1                       @ Save thread to schedule

    @ Setup a pointer to the thread control block. 
@    REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread;

        LDR     r1,Current_Thread           @ Pickup current thread ptr address

    @ Clear the current thread control block. 
@    TCD_Current_Thread =  NU_NULL;

        LDR     r3,Slice_State              @ Pickup time slice state address
        STR     r2,[r1, #0]                 @ Set current thread pointer to 
                                            @ NU_NULL

    @ Check to see if a time slice is active.  If so, copy the original time
    @ slice into the current time slice field of the task's control block.
@    if (TMD_Time_Slice_State == 0)
@    {

        LDR     r1,[r3, #0]                 @ Pickup time slice state flag
        CMP     r1,#0                       @ Compare with active value
        BNE     TCT_No_Stop_TS_4            @ If non-active, don't disable

    @ Insure that the next time the task runs it gets a fresh time 
    @ slice. 
@       REG_Thread_Ptr -> tc_cur_time_slice =  REG_Thread_Ptr -> tc_time_slice;

        LDR     r1,[r0, #0x40]              @ Pickup original time slice

    @ Clear any active time slice by setting the state to NOT_ACTIVE. 
@        TMD_Time_Slice_State =  1;

        MOV     r2,#1                       @ Build disable value
        STR     r2,[r3, #0]                 @ Disable time slice
        STR     r1,[r0, #0x20]              @ Reset current time slice

@    }
TCT_No_Stop_TS_4:

    @ Save off the current stack pointer in the control block. 
@    REG_Thread_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr;

        STR     r13,[r0, #0x2c]             @ Save the thread's stack pointer

    @ Switch to the system stack. 
@    TCD_System_Stack =  (VOID *) REG_Stack_Ptr;

        LDR     r1,System_Stack             @ Pickup address of stack pointer
        LDR     r2,System_Limit             @ Pickup address of stack limit ptr
        LDR     r13,[r1, #0]                @ Switch to system stack
        LDR     r10,[r2, #0]                @ Setup system stack limit

    @ Transfer control to the specified thread directly.
@    TCT_Control_To_Thread(thread);

        LDR     r2,Int_Level                @ Pickup address of interrupt level
        MRS     r1,CPSR                     @ Pickup current CPSR
        LDR     r3,[r2, #0]                 @ Pickup interrupt lockout level
        BIC     r1,r1,#LOCK_MSK             @ Clear lockout bits
        ORR     r1,r1,r3                    @ Build new interrupt lockout
        MOV     r0,r4                       @ Indicate thread to schedule
        MSR     CPSR,r1                     @ Setup CPSR appropriately
        ORR     r1,r1,#LOCKOUT              @ Build lockout value again
        MSR     CPSR,r1                     @ Lockout interrupts again
        B       TCT_Control_To_Thread       @ Schedule the thread indirectly
@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Interrupt_Context_Save                                       
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function saves the interrupted thread's context.  Nested    
 *      interrupts are also supported.  If a task or HISR thread was     
 *      interrupted, the stack pointer is switched to the system stack   
 *      after the context is saved.                                      
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Application ISRs                    Assembly language ISRs       
 *      INT_Interrupt_Shell                 Interrupt handler shell      
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      vector                              Interrupt's vector number    
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      None                                                             
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *      D. Driscoll     01-04-2002      Released version 1.13.3.         
 *                                      Updated to handle nested /       
 *                                      prioritized IRQs                 
 ************************************************************************
 */

@VOID  TCT_Interrupt_Context_Save(INT vector)
@{

	.globl  TCT_Interrupt_Context_Save
TCT_Interrupt_Context_Save:
        @ Determine if this is a nested interrupt. 
        LDR     r1,Int_Count                @ Pickup address of interrupt count
        LDR     r2,[r1, #0]                 @ Pickup interrupt counter
        ADD     r2,r2,#1                    @ Add 1 to interrupt counter
        STR     r2,[r1, #0]                 @ Store new interrupt counter value
        CMP     r2,#1                       @ Is it nested?
        BEQ     TCT_Not_Nested_Save         @ No

@ Nested interrupt.  Save complete context on the current stack. 
TCT_Nested_Save:

/* No longer needed in the FreeCalypso version, as we can use r0 instead. */
#if 0
@       1.  Save another register on the exception stack so we have enough to work with
        STMDB   r13!,{r5}
#endif

@       2.  Save the necessary exception registers into r1-r3
        MOV     r1,r13                      @ Put the exception r13 into r1
        MOV     r2,r14                      @ Move the return address for the caller
                                            @  of this function into r2
        MRS     r3,spsr                     @ Put the exception spsr into r3

@       3.  Adjust the exception stack pointer for future exceptions
        ADD     r13,r13,#20                 @ r13 reset to pre-interrupt value

@       4.  Switch CPU modes to save context on system stack
        MRS     r0,CPSR                     @ Pickup the current CPSR
        BIC     r0,r0,#MODE_MASK            @ Clear the mode bits
        
        ORR     r0,r0,#SUP_MODE             @ Change to supervisor mode (SVD)
        
        MSR     CPSR,r0                     @ Switch modes (IRQ->SVC)

@       5.  Store the SVC r13 into r5 so the r13 can be saved as is.
@	FreeCalyspo: using r0 instead
        MOV     r0,r13

@       6.  Save the exception return address on the stack (r15).
        STMDB   r0!,{r4}

@       7.  Save r5-r14 on stack (used to be r6-r14)
        STMDB   r0!,{r5-r14}

@       8.  Switch back to using r13 now that the original r13 has been saved.
        MOV     r13,r0

/* no longer relevant */
#if 0
@       9.  Get r5 and exception enable registers off of exception stack and
@              save r5 (stored in r4) back to the system stack.
        LDMIA   r1!,{r4-r5}
        STMDB   r13!,{r4}
        MOV     r4,r5                       @ Put exception enable value into r4
#endif

@       10. Get the rest of the registers off the exception stack and
@              save them onto the system stack.
        LDMIA   r1!,{r5-r8,r11}             @ Get r0-r4 off exception stack
        STMDB   r13!,{r5-r8,r11}            @ Put r0-r4 on system stack

/* no longer relevant */
#if 0
@       11. Store the exception enable value back on the exception stack.
        STMDB   r1,{r4}
#endif

@       12. Save the SPSR on the system stack (CPSR)
        STMDB   r13!,{r3}

/* TI's approach to interrupt handling does not support re-enabling here */
#if 0
@       13. Re-enable interrupts
        MRS     r1,CPSR
        BIC     r1,r1,#(IRQ_BIT_OR_FIQ_BIT)
        MSR     CPSR,r1
#endif

        BX      r2                          @ Return to calling ISR
@    }
@    else
@    {
TCT_Not_Nested_Save:

        @ Determine if a thread was interrupted. 
@       if (TCD_Current_Thread)
@       {

        LDR     r1,Current_Thread           @ Pickup current thread ptr address
        LDR     r1,[r1, #0]                 @ Pickup the current thread pointer
        CMP     r1,#0                       @ Is it NU_NULL?
        BEQ     TCT_Idle_Context_Save       @ If no, no real save is necessary


        @ Yes, a thread was interrupted.  Save complete context on the
        @ thread's stack. 

/* No longer needed in the FreeCalypso version, as we can use r0 instead. */
#if 0
@       1.  Save another register on the exception stack so we have enough to work with
        STMDB   r13!,{r5}
#endif

@       2.  Save the necessary exception registers into r1-r3
        MOV     r1,r13                      @ Put the exception r13 into r1
        MOV     r2,r14                      @ Move the return address for the caller
                                            @  of this function into r2
        MRS     r3,spsr                     @ Put the exception spsr into r3

@       3.  Adjust the exception stack pointer for future exceptions
        ADD     r13,r13,#20                 @ r13 reset to pre-interrupt value

@       4.  Switch CPU modes to save context on system stack
        MRS     r0,CPSR                     @ Pickup the current CPSR
        BIC     r0,r0,#MODE_MASK            @ Clear the mode bits
        
        ORR     r0,r0,#SUP_MODE             @ Change to supervisor mode (SVD)
        
        MSR     CPSR,r0                     @ Switch modes (IRQ->SVC)

@       5.  Store the SVC r13 into r5 so the r13 can be saved as is.
@	FreeCalyspo: using r0 instead
        MOV     r0,r13

@       6.  Save the exception return address on the stack (r15).
        STMDB   r0!,{r4}

@       7.  Save r5-r14 on stack (used to be r6-r14)
        STMDB   r0!,{r5-r14}

@       8.  Switch back to using r13 now that the original r13 has been saved.
        MOV     r13,r0

/* no longer relevant */
#if 0
@       9. Get r5 and exception enable registers off of exception stack and
@          save r5 (stored in r4) back to the system stack.
        LDMIA   r1!,{r4-r5}
        STMDB   r13!,{r4}
        MOV     r4,r5                       @ Put exception enable value into r4
#endif

@       10. Get the rest of the registers off the exception stack and
@           save them onto the system stack.
        LDMIA   r1!,{r5-r8,r11}             @ Get r0-r4 off exception stack
        STMDB   r13!,{r5-r8,r11}            @ Put r0-r4 on system stack

/* no longer relevant */
#if 0
@       11. Store the exception enable value back on the exception stack.
        STMDB   r1,{r4}
#endif

@       12. Save the SPSR on the system stack (CPSR)
        STMDB   r13!,{r3}

@       13. Save stack type to the task stack (1=interrupt stack)
        MOV     r1,#1                       @ Interrupt stack type
        STMDB   r13!,{r1}

        @ Save the thread's stack pointer in the control block. 
@       REG_Thread_Ptr =  (TC_TCB *) TCD_Current_Thread
@       REG_Thread_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr

        LDR     r1,Current_Thread           @ Pickup current thread ptr address
        LDR     r3,[r1, #0]                 @ Pickup current thread pointer
        STR     r13,[r3, #TC_STACK_POINTER] @ Save stack pointer

        @ Switch to the system stack. 
@            REG_Stack_Ptr =  TCD_System_Stack

        LDR     r1,System_Stack             @ Pickup address of stack pointer
        LDR     r3,System_Limit             @ Pickup address of stack limit ptr
        LDR     r13,[r1, #0]                @ Switch to system stack
        LDR     r10,[r3, #0]                @ Setup system stack limit

/* TI's approach to interrupt handling does not support re-enabling here */
#if 0
        @ Re-enable interrupts
        MRS     r1,CPSR
        BIC     r1,r1,#(IRQ_BIT_OR_FIQ_BIT)
        MSR     CPSR,r1
#endif
 
@ Return to caller ISR. 

        BX      r2                          @ Return to caller ISR

@       }

TCT_Idle_Context_Save:

        MOV     r2,r14                      @ Save r14 in r2
@       LDR     r3,[r13]                    @ Get exception enable value from stack
        ADD     r13,r13,#20                 @ Adjust exception r13 for future interrupts
@       STR     r3,[r13]                    @ Put exception enable value back on stack

        MRS     r1,CPSR                     @ Pickup current CPSR
        BIC     r1,r1,#MODE_MASK            @ Clear the current mode
@       BIC     r1,r1,#(IRQ_BIT_OR_FIQ_BIT) @ Re-enable interrupts
        
        ORR     r1,r1,#SUP_MODE             @ Prepare to switch to supervisor
                                            @   mode (SVC)
        MSR     CPSR,r1                     @ Switch to supervisor mode (SVC)

        BX      r2                          @ Return to caller ISR

@    }
@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Interrupt_Context_Restore                                    
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function restores the interrupt context if a nested         
 *      interrupt condition is present.  Otherwise, this routine         
 *      transfers control to the scheduling function.                    
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Application ISRs                    Assembly language ISRs       
 *      INT_Interrupt_Shell                 Interrupt handler shell      
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      TCT_Schedule                        Thread scheduling function   
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
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *      D. Driscoll     01-04-2002      Released version 1.13.3.         
 *                                      Updated to handle nested /       
 *                                      prioritized IRQs                 
 *                                                                       
 ************************************************************************
 */

@VOID  TCT_Interrupt_Context_Restore(void)
@{

	.globl  TCT_Interrupt_Context_Restore
TCT_Interrupt_Context_Restore:
    @ It is assumed that anything pushed on the stack by ISRs has been
    @ removed upon entry into this routine. 

    @ Decrement and check for nested interrupt conditions. 
@    if (--TCD_Interrupt_Count)
@    {

        LDR     r1,Int_Count                @ Pickup address of interrupt count
        LDR     r2,[r1, #0]                 @ Pickup interrupt counter
        SUB     r2,r2,#1                    @ Decrement interrupt counter
        STR     r2,[r1, #0]                 @ Store interrupt counter
        CMP     r2,#0
        BEQ     TCT_Not_Nested_Restore

        @ Restore previous context. 

        LDR     r1,[r13], #4                @ Pickup the saved CPSR
        
        MSR     SPSR,r1                     @ Place into saved SPSR
        LDMIA   r13,{r0-r15}^               @ Return to the point of interrupt

@    }
@    else
@    {

TCT_Not_Nested_Restore:

        @ Determine if a thread is active. 
@        if (TCD_Current_Thread)
@        {

        LDR     r1,Current_Thread           @ Pickup current thread ptr address
        LDR     r0,[r1, #0]                 @ Pickup current thread pointer
        CMP     r0,#0                       @ Determine if a thread is active
        BEQ     TCT_Idle_Context_Restore    @ If not, idle system restore

        @ Clear the current thread pointer. 
@            TCD_Current_Thread =  NU_NULL

        MOV     r2,#0                       @ Build NU_NULL value
        STR     r2,[r1, #0]                 @ Set current thread ptr to NU_NULL

        @ Determine if a time slice is active.  If so, the remaining
        @ time left on the time slice must be saved in the task's
        @ control block. 
@            if (TMD_Time_Slice_State == 0)
@            {

        LDR     r3,Slice_State              @ Pickup time slice state address
        LDR     r1,[r3, #0]                 @ Pickup time slice state
        CMP     r1,#0                       @ Determine if time slice active
        BNE     TCT_Idle_Context_Restore    @ If not, skip time slice reset

        @ Pickup the remaining portion of the time slice and save it
        @ in the task's control block. 
@                REG_Thread_Ptr -> tc_cur_time_slice =  TMD_Time_Slice
@                TMD_Time_Slice_State =  1

        LDR     r2,Time_Slice               @ Pickup address of time slice left
        MOV     r1,#1                       @ Build disable time slice value
        LDR     r2,[r2, #0]                 @ Pickup remaining time slice
        STR     r1,[r3, #0]                 @ Disable time slice
        STR     r2,[r0, #TC_CUR_TIME_SLICE] @ Store remaining time slice

@            }
@        }
TCT_Idle_Context_Restore:

        @ Reset the system stack pointer. 
        LDR     r1,System_Stack             @ Pickup address of stack pointer
        LDR     r2,System_Limit             @ Pickup address of stack limit ptr
        LDR     r13,[r1, #0]                @ Switch to system stack
        LDR     r10,[r2, #0]                @ Setup system stack limit

        @ Return to scheduler. 

        B       TCT_Schedule                @ Return to scheduling loop

@    }
@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_Activate_HISR                                                
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function activates the specified HISR.  If the HISR is      
 *      already activated, the HISR's activation count is simply         
 *      incremented.  Otherwise, the HISR is placed on the appropriate   
 *      HISR priority list in preparation for execution.                 
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      Application LISRs                                                
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      None                                                             
 *                                                                       
 * INPUTS                                                                
 *                                                                       
 *      hisr                                Pointer to HISR to activate  
 *                                                                       
 * OUTPUTS                                                               
 *                                                                       
 *      NU_SUCCESS                          Successful completion        
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@STATUS  TCT_Activate_HISR(TC_HCB *hisr)
@{

	.globl  TCT_Activate_HISR
TCT_Activate_HISR:

@INT     priority;


        @ Lockout interrupts. 

        STR     r4,[r13, #-4]!              @ Save r4
        MRS     r4,CPSR                     @ Pickup current CPSR
        ORR     r1,r4,#LOCKOUT              @ Build interrupt lockout value
        MSR     CPSR,r1                     @ Lockout interrupts

    @ Determine if the HISR is already active. 
@    if (hisr -> tc_activation_count)
@    {

        LDR     r1,[r0,#0x40]               @ Pickup current activation count
        CMP     r1,#0                       @ Is it the first activation?
        BEQ     TCT_First_Activate          @ Yes, place it on the correct list

    @ Increment the activation count.  Make sure that it does not go
    @ to zero. 
@        hisr -> tc_activation_count++;

        ADDS    r1,r1,#1                    @ Increment the activation count
        STR     r1,[r0,#0x40]               @ Store new activation count

@        if (hisr -> tc_activation_count == 0)

@            hisr -> tc_activation_count =  0xFFFFFFFFUL;

        MVNEQ   r1,#0                       @ If counter rolled-over reset
        STREQ   r1,[r0,#0x40]               @ Store all ones count
        B       TCT_Activate_Done           @ Finished with activation
@    }
@    else
@    {
TCT_First_Activate:


        @ Set the activation count to 1. 
@        hisr -> tc_activation_count =  1;

        MOV     r1,#1                       @ Initial activation count
        STR     r1,[r0,#0x40]               @ Store initial activation count

        @ Pickup the HISR's priority. 
@        priority =  hisr -> tc_priority;

        @ Determine if there is something in the given priority list. 
@        if (TCD_Active_HISR_Tails[priority])
@        {

        LDRB    r1,[r0,#0x1a]               @ Pickup priority of HISR
        LDR     r2,HISR_Tails               @ Pickup tail pointer base
        LDR     r3,[r2,r1,LSL #2]           @ Pickup tail pointer for priority
        CMP     r3,#0                       @ Is this first HISR at priority?
        BEQ     TCT_First_HISR              @ No, append to end of HISR list

        @ Something is already on this list.  Add after the tail. 
@            (TCD_Active_HISR_Tails[priority]) -> tc_active_next =  hisr;
@            TCD_Active_HISR_Tails[priority] =  hisr;

        STR     r0,[r3,#0x3c]               @ Setup the active next pointer
        STR     r0,[r2,r1,LSL #2]           @ Setup the tail pointer
        B       TCT_Activate_Done           @ Finished with activate processing
@        }
@        else
@        {
TCT_First_HISR:

        @ Nothing is on this list. 
@           TCD_Active_HISR_Heads[priority] =  hisr;
@           TCD_Active_HISR_Tails[priority] =  hisr;

        LDR     r3,HISR_Heads               @ Pickup address of head pointers
        STR     r0,[r2,r1,LSL #2]           @ Set tail pointer to this HISR
        STR     r0,[r3,r1,LSL #2]           @ Set head pointer to this HISR

        @ Determine the highest priority HISR. 
@           if (TCD_Active_HISR_Heads[0])
@               TCD_Execute_HISR =  TCD_Active_HISR_Heads[0];
@           else if (TCD_Active_HISR_Heads[1])
@               TCD_Execute_HISR =  TCD_Active_HISR_Heads[1];
@           else
@               TCD_Execute_HISR =  TCD_Active_HISR_Heads[2];

        LDR     r1,[r3,#0]                  @ Pickup priority 0 head pointer
        LDR     r0,Execute_HISR             @ Build address to execute HISR ptr
        CMP     r1,#0                       @ Is priority 0 active?
        LDREQ   r1,[r3,#4]                  @ If not, pickup priority 1 head 
        CMPEQ   r1,#0                       @ Is priority 1 active?
        LDREQ   r1,[r3,#8]                  @ Else, must be priority 2 active
        STR     r1,[r0,#0]                  @ Store which ever priority is the
                                            @   active one
@       }
@    }
TCT_Activate_Done:

        MSR     CPSR,r4                     @ Restore interrupt lockout
        LDR     r4,[r13], #4                @ Restore corrupted r4

@    return(NU_SUCCESS);

        MOV     r0,#0                       @ Always return NU_SUCCESS

        BX      r14                         @ Return to caller

@}

/*
 ************************************************************************
 *                                                                       
 * FUNCTION                                                              
 *                                                                       
 *      TCT_HISR_Shell                                                   
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This function is the execution shell of each and every HISR.  If 
 *      the HISR has completed its processing, this shell routine exits  
 *      back to the system.  Otherwise, it sequentially calls the HISR   
 *      routine until the activation count goes to zero.                 
 *                                                                       
 * CALLED BY                                                             
 *                                                                       
 *      HISR Scheduling                                                  
 *                                                                       
 * CALLS                                                                 
 *                                                                       
 *      hisr -> tc_entry                    Actual entry function of HISR
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
 *      W. Lamie        02-15-1994      Created initial version 1.0      
 *      D. Lamie        02-15-1994      Verified version 1.0             
 *                                                                       
 ************************************************************************
 */

@VOID    TCT_HISR_Shell(void)
@{
        .globl  TCT_HISR_Shell
TCT_HISR_Shell:

    @ Point at the HISR. 
@    REG_HISR_Ptr =  (TC_HCB *) TCD_Current_Thread;

        LDR     r0,Current_Thread            @ Build address of thread pointer
        LDR     r5,[r0, #0]                  @ Pickup control block pointer

@    do
@    {
TCT_HISR_Loop:

        @ Call the HISR's entry routine. 
@        (*(REG_HISR_Ptr -> tc_entry)) ();

/* old TMS470 code:

        .if THUMB = 0

         MOV     r14,r15                     ; Setup return value
         LDR     r15,[r5,#44h]               ; Call HISR entry function
        .else
         LDR     r4,[r5,#44h]                ; Get HISR entry function
         TST     r4,#1                       ; See if calling Thumb or ARM
         BNE     Thumbsec
         MOV     r14,r15                     ; Setup return value
         BX      r4
         B       ARMCODE
Thumbsec:
         ADD     r14, r15, #1 
         BX      r4
         .state16
ThumbAfterHisr
         MOV     r1, r15  
         BX      r1
        .state32
        .endif
*/

	/* new code for the GNU style of ARM/Thumb interworking */
	ldr	r4, [r5, #TC_HISR_ENTRY]
	mov	lr, pc
	bx	r4

        @ Lockout interrupts. 

        MRS     r1,CPSR                     @ Pickup current CPSR
        ORR     r1,r1,#LOCKOUT              @ Build interrupt lockout 
        MSR     CPSR,r1                     @ Lockout interrupts

        @ On return, decrement the activation count and check to see if 
        @ it is 0.  Once it reaches 0, the HISR should be made inactive.
@        REG_HISR_Ptr -> tc_activation_count--;


        LDR     r0,[r5, #0x40]              @ Pickup current activation count
        SUBS    r0,r0,#1                    @ Subtract and set condition codes
        STR     r0,[r5, #0x40]              @ Store new activation count
        BEQ     TCT_HISR_Finished           @ Finished processing HISR

        @ Restore interrupts. 

        LDR     r2,Int_Level                @ Pickup address of interrupt level
        MRS     r1,CPSR                     @ Pickup current CPSR
        LDR     r3,[r2, #0]                 @ Pickup interrupt lockout level
        BIC     r1,r1,#LOCK_MSK             @ Clear lockout bits
        ORR     r1,r1,r3                    @ Build new interrupt lockout
        MSR     CPSR,r1                     @ Setup CPSR appropriately
        B       TCT_HISR_Loop               @ Return to HISR loop
@    }
@    while (REG_HISR_Ptr -> tc_activation_count);

TCT_HISR_Finished:

        @ At this point, the HISR needs to be made inactive. 

        @ Determine if this is the only HISR on the given priority list. 
@    if (REG_HISR_Ptr == TCD_Active_HISR_Tails[REG_HISR_Ptr -> tc_priority])
@    {

        LDR     r14,HISR_Tails               @ Pickup tail pointers address
        LDRB    r3,[r5,#0x1a]                @ Pickup priority
        LDR     r6,[r14,r3,LSL #2]           @ Pickup this priority tail pointer
        LDR     r2,Execute_HISR              @ Build address to execute HISR ptr
        MOV     r12,#0                       @ Clear r12
        LDR     r1,HISR_Heads                @ Pickup head pointers address
        CMP     r6,r5                        @ Is this priority tail the same as
                                             @ the current HISR?
        BNE     TCT_More_HISRs               @ If not, more HISRs at this 
                                             @ priority

        @ The only HISR on the list.  Clean up the list and check for the
        @ highest priority HISR. 
@       TCD_Active_HISR_Heads[REG_HISR_Ptr -> tc_priority] =  NU_NULL;
@       TCD_Active_HISR_Tails[REG_HISR_Ptr -> tc_priority] =  NU_NULL;

        STR     r12,[r1,r3,LSL #2]            @ Set head pointer to NU_NULL
        STR     r12,[r14,r3,LSL #2]           @ Set tail pointer to NU_NULL

        @ Determine the highest priority HISR. 
@       if (TCD_Active_HISR_Heads[0])
@           TCD_Execute_HISR =  TCD_Active_HISR_Heads[0];
@       else if (TCD_Active_HISR_Heads[1])
@           TCD_Execute_HISR =  TCD_Active_HISR_Heads[1];
@       else
@           TCD_Execute_HISR =  TCD_Active_HISR_Heads[2];

        LDR     r3,[r1,#0]                    @ Pickup priority 0 head pointer
        CMP     r3,#0                         @ Is there an HISR active?
        LDREQ   r3,[r1,#4]                    @ If not, pickup priority 1 pointer
        CMPEQ   r3,#0                         @ Is there an HISR active?
        LDREQ   r3,[r1,#8]                    @ If not, pickup priority 2 pointer
        STR     r3,[r2,#0]                    @ Setup execute HISR pointer
        B       TCT_HISR_Exit                 @ Exit HISR processing
@    }
@    else
@    {

TCT_More_HISRs:

        @ Move the head pointer to the next HISR in the list. 
@        TCD_Active_HISR_Heads[REG_HISR_Ptr -> tc_priority] =  
@                                        REG_HISR_Ptr -> tc_active_next;

        @ Also set the TCD_Execute_HISR pointer. 
@        TCD_Execute_HISR =  REG_HISR_Ptr -> tc_active_next;

        LDR     r14,[r5,#0x3c]                @ Pickup next HISR to activate
        STR     r14,[r1,r3,LSL #2]            @ Setup new head pointer
        STR     r14,[r2, #0]                  @ Setup execute HISR pointer
@    }

TCT_HISR_Exit:

        @ Build fake return to the top of this loop.  The next time the HISR
        @ is activated, it will return to the top of this function. 

        LDR     r14,HISR_Shell                @ Pickup address of shell entry
        STMDB   r13!,{r4-r12,r14}             @ Save minimal context of thread on
                                              @ the current stack
        MOV     r2,#0                         @ Build solicited stack type value
                                              @ and NU_NULL value
#if 1	/* was .if THUMB */
        STR     r2,[r13, #-4]!                @ Save state mask
#endif
        STR     r2,[r13, #-4]!                @ Place it on the top of the stack

        @ Clear the current thread control block. 
@    TCD_Current_Thread =  NU_NULL;

        LDR     r1,Current_Thread            @ Pickup current thread ptr address
        STR     r2,[r1, #0]                  @ Set current thread pointer to 
                                             @   NU_NULL

        @ Save off the current stack pointer in the control block. 
@    REG_HISR_Ptr -> tc_stack_pointer =  (VOID *) REG_Stack_Ptr;

         
        STR     r13,[r5, #0x2c]              @ Save the thread's stack pointer


        @ Switch to the system stack. 
@    REG_Stack_Ptr =  (BYTE_PTR) TCD_System_Stack;

        LDR     r1,System_Stack              @ Pickup address of stack pointer
        LDR     r2,System_Limit              @ Pickup address of stack limit ptr
        LDR     r13,[r1, #0]                 @ Switch to system stack
        LDR     r10,[r2, #0]                 @ Setup system stack limit

        @ Transfer control to the main scheduling loop. 

        B       TCT_Schedule                 @ Return to main scheduling loop
@}

/* FreeCalypso addition, used by riviera/rvf/rvf_task.c: */
	.globl	INT_Check_IRQ_Mask
INT_Check_IRQ_Mask:
	MRS	r0,CPSR
	BX	lr

