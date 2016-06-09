/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccd_globs.h
+----------------------------------------------------------------------------- 
|  Copyright 2002 Texas Instruments Berlin, AG 
|                 All rights reserved. 
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Berlin, AG 
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Berlin, AG. 
+----------------------------------------------------------------------------- 
|  Purpose :  Condat Coder Decoder
|             Definition of C struct containing ccd internal global variables
+----------------------------------------------------------------------------- 
*/ 

#ifndef __CCD_GLOBS_H
#define __CCD_GLOBS_H

#include <setjmp.h>
#include "ccdtable.h"
/*
 * Constants needed for ccd_globs.h
 */
#include "ccdapi.h"

#if !defined (CCDDATA_DYN) && !defined (CCD_TEST)
#include "vsi.h"
#endif

/*
 * stack size for the UPN calculator
 */
#define MAX_UPN_STACK_SIZE 20
#define MAX_KEEP_REG_SIZE  15

/*
 * the two constants give the size of the iei table
 */
#define MAX_IE_PER_MSG         40
#define MAX_RECURSIONS_PER_MSG 8

#ifdef CCD_GPRS_ONLY
#define CCD_ERR_STK_SZ (MAX_RECURSIONS_PER_MSG+2)
#else
#define CCD_ERR_STK_SZ 50 
#endif

/*
 * constants and types needed for error handling
 */
#define MAX_ERRORS 10 

#define ENCODE_FUN   0
#define DECODE_FUN   1

/*
 * declare a table for the iei processing. This table
 * contains for each iei a low and high boundary of
 * valid repeats and the actual number of repeats
 */
typedef struct
{
  unsigned   valid:4;
  unsigned   multiple:2;
  unsigned   exhausted:2; /* for GSM1_ASN elements */
/*  BOOL   choice;   version does not use GSM1_ASNCHC */
/*  UBYTE  min_amount; seams to be an unused variable */
  UBYTE  max_amount;
  UBYTE  act_amount;
  UBYTE  ident;
} T_IEI_TABLE;


/*
 * for each msg an initialisation of e.g. the iei_table is to
 * perform. See cdc_GSM_start();
 */

typedef struct
{
  unsigned      valid:4;
  unsigned      EOCPending:4; /* for ASN1-BER elements only */
  UBYTE         countSkipped; /* for GSM1_ASN elements */
  USHORT        melemStart;
  USHORT        melemLast;
  USHORT        ieTableLen;
  T_IEI_TABLE   iei_table[MAX_IE_PER_MSG];
} T_IEI_CONTEXT;



typedef struct
{
#if defined (CCDDATA_DYN) || defined (CCD_TEST)
  int me;
#else
  T_HANDLE me;        /* entity calling CCD */
#endif
  SHORT   CCD_Error;  /* return variable overwritten by ccd_setError */

  /*
   * variable used for processing of nested information elements
   */
  UBYTE   ccd_recurs_level;

  /*
   * ccd uses setjmp() and longjmp to process some error cases.
   */
  BOOL    jmp_mark_set;
  jmp_buf jmp_mark;

#ifdef DYNAMIC_ARRAYS
  /*
   * Pointer to head of allocation chain for primitives with pointer types.
   */
  U8     *alloc_head;
#endif

  /*
   * variables used for bit buffering and manipulation of a message
   */
  UBYTE  *bitbuf;
  UBYTE  *pstruct;
  ULONG   pstructOffs;
  USHORT  bitpos;
  USHORT  bytepos;
  USHORT  buflen;
  USHORT  bitoffs;
  USHORT  lastbytepos16;
  USHORT  lastbytepos32;
  USHORT  maxBitpos;
  UBYTE   byteoffs;
  /*
   * variables used by the UPN caculator 
   */
  UBYTE   SP;
  ULONG   Stack[MAX_UPN_STACK_SIZE];
  ULONG   KeepReg[MAX_KEEP_REG_SIZE];
  BOOL    StackOvfl;

  /* 
   * variable used when detecting unknown extensions 
   * of IEs of type CCDTYPE_GSM5_TLV.
   */
  BOOL    SeekTLVExt;

  /*
   * variables used as cash to keep data on octet boundaries
   */
  USHORT  last16Bit;
  ULONG   last32Bit;

#ifdef ERR_TRC_STK_CCD
  U16     error_stack[CCD_ERR_STK_SZ];/*??*/
#endif /* ERR_TRC_STK_CCD */
  U8      errLabel;
  U8      continue_array;
  U16     msgLen;

  /*
   * variables used by the modules in cdc_gsm.c 
   */
  T_IEI_CONTEXT iei_ctx[MAX_RECURSIONS_PER_MSG];  
  USHORT  RefBeforeError;
  USHORT  akt1VPos;
  USHORT  next1VPos;
  BOOL    TagPending; 
  UBYTE   PendingTag; 
  BOOL    SequenceError;
  BOOL    Swap1V_inProgress;
  UBYTE   last_level;
  UBYTE   numEOCPending;

#ifdef DEBUG_CCD
  BOOL    TraceIt;
  char    buf[33];
#endif

} T_CCD_Globs;

#endif
