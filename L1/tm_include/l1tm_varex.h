 /************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1TM_VAREX.H
 *
 *        Filename l1tm_varex.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/


#ifdef L1TM_ASYNC_C //Defined in l1_tmode.c
  #define TMVAR
#else
  #define TMVAR extern
#endif

TMVAR T_L1TM_GLOBAL  l1tm;

