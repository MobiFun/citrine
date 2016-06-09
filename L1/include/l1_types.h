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
#if !defined (BOOL_FLAG)
  #define BOOL_FLAG
  typedef unsigned char  BOOL;
#endif


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

typedef volatile UWORD16  API;
typedef volatile WORD16   API_SIGNED;



