/************* Revision Controle System Header *************
 *                  GSM Layer 1 software 
 * L1_TYPES.H
 *
 *        Filename l1_types.h
 *  Copyright 2003 (C) Texas Instruments  
 *
 ************* Revision Controle System Header *************/

//--------------------------------------
// Basic DATA types used along L1 code.
//--------------------------------------

#ifndef __L1_TYPES_H__
  #define __L1_TYPES_H__

#if !defined (BOOL_FLAG)
  #define BOOL_FLAG
  typedef unsigned char  BOOL;
#endif /* #if !defined (BOOL_FLAG) */

// FreeCalypso change
#if 1 //(OP_L1_STANDALONE == 1)
 
 
#if !defined (NUCLEUS) && !defined CHAR_FLAG
  #define CHAR_FLAG

  typedef          char  CHAR;
#endif

typedef unsigned char  UWORD8;
typedef signed   char  WORD8;

typedef unsigned short UWORD16;
typedef          short WORD16;

typedef unsigned long  UWORD32;
typedef          long  WORD32;
//--------------------------------------
 

#else
  #include "global_types.h"
#endif /* #if (OP_L1_STANDALONE == 1) */





typedef volatile UWORD16  API;
typedef volatile WORD16   API_SIGNED;
#endif /* #ifndef __L1_TYPES_H__ */

