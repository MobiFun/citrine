/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1_API_HISR.H
 *
 *        Filename l1_api_hisr.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

#ifndef _L1_API_HISR_H_
#define _L1_API_HISR_H_

/* Constants */
#define ID_API_INT		0x4

/* Prototypes */
void l1_trigger_api_interrupt();
void l1_api_handler();

#endif
