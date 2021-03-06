/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   : ccdtable.h
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
|  Purpose :  Condat Conder Decoder
|             Type definitions for the coding/decoding tables
|             The tables are generated by CCDGEN.
+----------------------------------------------------------------------------- 
*/ 


#ifndef __CCDTABLE
#define __CCDTABLE

#define CCDDATA_TABLE_VERSION 3

#ifdef WIN32
  #ifndef CCDDATA_U32
  #define CCDDATA_U32
  #endif
#endif

/*
 * Table entry for a variable
 */
typedef struct
{
#ifdef CCD_SYMBOLS
  char     *name;
  USHORT    longNameRef;
#endif
  USHORT    bSize;
  USHORT    cSize;
  char      cType;
  UBYTE     numValueDefs;
  USHORT    valueDefs;
} T_CCD_VarTabEntry;

/*
 * Table entry for a spare
 */
typedef struct
{
  ULONG     value;
  UBYTE     bSize;
} T_CCD_SpareTabEntry;

/*
 * Table entry for an element
 */
typedef struct
{
  UBYTE     codingType;
  BOOL      optional;
  char      extGroup;
  char      repType;
  USHORT    calcIdxRef;
  USHORT    maxRepeat;
#ifdef CCDDATA_U32
  U32       structOffs;
#else /* CCDDATA_U32 */
  USHORT    structOffs;
#endif /* CCDDATA_U32 */
  USHORT    ident;
  char      elemType;
  USHORT    elemRef;
} T_CCD_ElemTabEntry;

/*
 * Table entry for a calculation index
 */
typedef struct
{
  UBYTE     numCondCalcs;
  USHORT    condCalcRef;
  UBYTE     numPrologSteps;
  USHORT    prologStepRef;
  USHORT    numRepCalcs;
  USHORT    repCalcRef;
} T_CCD_CalcIndex;


/*
 * Definition entry for a composition
 */
typedef struct
{
#ifdef CCD_SYMBOLS
  char     *name;
  USHORT    longNameRef;
#endif
#ifdef CCDDATA_U32
  U32       cSize;
  U32       bSize;
#else /* CCDDATA_U32 */
  USHORT    cSize;
  USHORT    bSize;
#endif /* CCDDATA_U32 */
  USHORT    numOfComponents;
  USHORT    componentRef;
} T_CCD_CompTabEntry;

/*
 * Definition entry for a calculation
 */
typedef struct
{
  char      operation;
  U16       operand;
} T_CCD_CalcTabEntry;

/*
 * Definition entry for a value
 */
typedef struct
{
  USHORT    valStringRef;
  UBYTE      isDefault;
  S32     startValue;
  S32     endValue;
} T_CCD_ValTabEntry;

typedef char * T_CCD_StrTabEntry;

typedef struct
{
  char* as_name;
} T_CCD_ALIASTABLE;

typedef struct
{
  USHORT  numitems;
  USHORT  idx;
} T_CCD_MTXIDX;

#define NO_REF 0xffff

#endif
