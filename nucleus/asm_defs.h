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
 * FILE NAME                            VERSION                          
 *                                                                       
 *      asm_defs.inc            Nucleus PLUS\ARM925\Code Composer 1.14.1 
 *                                                                       
 * COMPONENT                                                             
 *                                                                       
 *      IN - Initialization                                              
 *                                                                       
 * DESCRIPTION                                                           
 *                                                                       
 *      This file contains the target processor dependent initialization 
 *      values used in int.s, tct.s, and tmt.s                      
 *                                                                       
 * HISTORY                                                               
 *                                                                       
 *         NAME            DATE                    REMARKS               
 *                                                                       
 *      B. Ronquillo     08-28-2002          Released version 1.13.1      
 *                                                                       
 ************************************************************************
 */

/*
 **********************************
 * BOARD INITIALIZATION CONSTANTS *
 **********************************
 * Begin define constants used in low-level initialization.
 */

/* CPSR control byte definitions */
#define	LOCKOUT			0xC0	/* Interrupt lockout value */
#define	LOCK_MSK		0xC0	/* Interrupt lockout mask value */
#define	MODE_MASK		0x1F	/* Processor Mode Mask */
#define	SUP_MODE		0x13	/* Supervisor Mode (SVC) */
#define	IRQ_MODE		0x12	/* Interrupt Mode (IRQ) */
#define	IRQ_MODE_OR_LOCKOUT	0xD2	/* Combined IRQ_MODE OR'ed with */
					/* LOCKOUT */
#define	FIQ_MODE		0x11	/* Fast Interrupt Mode (FIQ) */
#define	IRQ_BIT			0x80	/* Interrupt bit of CPSR and SPSR */
#define	FIQ_BIT			0x40	/* Interrupt bit of CPSR and SPSR */
#define	IRQ_BIT_OR_FIQ_BIT	0xC0	/* IRQ or FIQ interrupt bit of CPSR */
					/* and SPSR */
#define	ABORT_MODE		0x17
#define	UNDEF_MODE		0x1B

/*
 ********************************************
 *  TC_TCB and TC_HCB STRUCT OFFSET DEFINES *
 ********************************************
 */
#define	TC_CREATED		0x00	/* Node for linking to created task */
					/* list */
#define	TC_ID			0x0C	/* Internal TCB ID */
#define	TC_NAME			0x10	/* Task name */
#define	TC_STATUS		0x18	/* Task status */
#define	TC_DELAYED_SUSPEND	0x19	/* Delayed task suspension */
#define	TC_PRIORITY		0x1A	/* Task priority */
#define	TC_PREEMPTION		0x1B	/* Task preemption enable */
#define	TC_SCHEDULED		0x1C	/* Task scheduled count */
#define	TC_CUR_TIME_SLICE	0x20	/* Current time slice */
#define	TC_STACK_START		0x24	/* Stack starting address */
#define	TC_STACK_END		0x28	/* Stack ending address */
#define	TC_STACK_POINTER	0x2C	/* Task stack pointer */
#define	TC_STACK_SIZE		0x30	/* Task stack's size */
#define	TC_STACK_MINIMUM	0x34	/* Minimum stack size */
#define	TC_CURRENT_PROTECT	0x38	/* Current protection */
#define	TC_SAVED_STACK_PTR	0x3C	/* Previous stack pointer */
#define	TC_ACTIVE_NEXT		0x3C	/* Next activated HISR */
#define	TC_TIME_SLICE		0x40	/* Task time slice value */
#define	TC_ACTIVATION_COUNT	0x40	/* Activation counter */
#define	TC_HISR_ENTRY		0x44	/* HISR entry function */
#define	TC_HISR_SU_MODE		0x58	/* Sup/User mode indicator for HISRs */
#define	TC_HISR_MODULE		0x5C	/* Module identifier for HISR's */
#define	TC_SU_MODE		0xA8	/* Sup/User mode indicator for Tasks */
#define	TC_MODULE		0xAC	/* Module identifier for Tasks */
