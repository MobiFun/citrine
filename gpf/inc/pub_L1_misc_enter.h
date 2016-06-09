/* 
+------------------------------------------------------------------------------
|  File:       pub_L1_misc_enter.h
+------------------------------------------------------------------------------
|  Copyright 2004 Texas Instruments Deutschland GmbH
|                 All rights reserved.
| 
|                 This file is confidential and a trade secret of Texas 
|                 Instruments Deutschland GmbH.
|                 The receipt of or possession of this file does not convey 
|                 any rights to reproduce or disclose its contents or to 
|                 manufacture, use, or sell anything it may describe, in 
|                 whole, or in part, without the specific written consent of 
|                 Texas Instruments Deutschland GmbH.
+------------------------------------------------------------------------------
| Purpose:    Check the SrcFileTime of the ccdgen generated header files at link time.
|
|             The macros in this file help to detect version mismatch between 
|             header files included for build of different libraries. 
|             Example of a linker error: 
|             xx_tdc_3_constraints.obj : error LNK2001: 
|             unresolved external symbol 
|             "char  BadLibVersionCheck____P_XX_TDC_3_VAL_H____Thu_Dec_12_17_49_56_2002" 
|             (?BadLibVersionCheck____P_XX_TDC_3_VAL_H____Thu_Dec_12_17_49_56_2002@@3DA)
|             \gpf\util\teststack\bin\test_xx_tdc\xx_tdc.dll : 
|                                  fatal error LNK1120: 1 unresolved externals
|             Error executing link.exe.
| 
|             The first group of macros (protected by PUB_L1_MISC_ENTER) are necessary 
|             to build the strings of type "BadLibVersionCheck__xxx".
|  
|             They need in turn other macros which are set in the *.h files of 
|             cdginc, tdcinc or message_san.h directory.
|             (e.g. PUB_L1_MISC_ENTER__M_XX_VAL_H__FILE_TYPE) 
|
|             The check is done only for the header files where 
|             ENABLE__PUB_L1_MISC_ENTER__SANITY_CHECK is switched on.
|
+------------------------------------------------------------------------------
*/

#ifndef PUB_L1_MISC_ENTER
#define PUB_L1_MISC_ENTER


/* we need a 2 stage approach to expand A and B before concatting them */
#define PUB_L1_MISC_ENTER_CONCATx(A,B) A##B
#define PUB_L1_MISC_ENTER_CONCAT(A,B) PUB_L1_MISC_ENTER_CONCATx(A,B)
#define PUB_L1_MISC_ENTER_CONCAT3x(A,B,C) A##B##C
#define PUB_L1_MISC_ENTER_CONCAT3(A,B,C) PUB_L1_MISC_ENTER_CONCAT3x(A,B,C)
#define PUB_L1_MISC_ENTER_CONCAT4x(A,B,C,D) A##B##C##D
#define PUB_L1_MISC_ENTER_CONCAT4(A,B,C,D) PUB_L1_MISC_ENTER_CONCAT4x(A,B,C,D)

/* we need a 2 stage approach to expand A before stringifying it */
#define PUB_L1_MISC_ENTER_STRINGx(A) #A
#define PUB_L1_MISC_ENTER_STRING(A) PUB_L1_MISC_ENTER_STRINGx(A)

#define PUB_L1_MISC_ENTER_GET_PRAGMA(PRAGMA) PUB_L1_MISC_ENTER_CONCAT3(PUB_L1_MISC_ENTER_,PUB_L1_MISC_ENTER__FILENAME,__##PRAGMA)


#define PUB_L1_MISC_ENTER_GET_FILE_TYPE PUB_L1_MISC_ENTER_CONCAT(PUB_L1_MISC_ENTER_FILE_TYPE__,PUB_L1_MISC_ENTER_GET_PRAGMA(FILE_TYPE))

#define PUB_L1_MISC_ENTER_SANITY_NAME PUB_L1_MISC_ENTER_CONCAT4(BadLibVersionCheck___,PUB_L1_MISC_ENTER__FILENAME,___,PUB_L1_MISC_ENTER_GET_PRAGMA(SRC_FILE_TIME))


/* create an enumerate sequence of the known file types so we can test which one we have with a '#if A == B' line */
#define PUB_L1_MISC_ENTER_FILE_TYPE__CDGINC 1
#define PUB_L1_MISC_ENTER_FILE_TYPE__TDCINC 2
#define PUB_L1_MISC_ENTER_FILE_TYPE__TDCINC_DSC 3
#define PUB_L1_MISC_ENTER_FILE_TYPE__TDCINC_MAIN 4
#endif 

/*
 * The check will be done only for files where 
 * ENABLE__PUB_L1_MISC_ENTER__SANITY_CHECK is switched on.
 */
#if (PUB_L1_MISC_ENTER_GET_FILE_TYPE == PUB_L1_MISC_ENTER_FILE_TYPE__TDCINC) 
#define ENABLE__PUB_L1_MISC_ENTER__SANITY_CHECK 
#endif

#ifdef PUB_L1_MISC_ENTER_DEBUG
#pragma message (PUB_L1_MISC_ENTER_STRING(PUB_L1_MISC_ENTER__FILENAME))
#pragma message (PUB_L1_MISC_ENTER_STRING(PUB_L1_MISC_ENTER_SANITY_NAME))
#pragma message (PUB_L1_MISC_ENTER_STRING(PUB_L1_MISC_ENTER_GET_FILE_TYPE))
#endif

#ifdef ENABLE__PUB_L1_MISC_ENTER__SANITY_CHECK 
  #ifdef PUB_L1_MISC_ENTER_DEFINE_SANITY

  char PUB_L1_MISC_ENTER_SANITY_NAME;
  #else
    #ifdef PUB_L1_MISC_ENTER_DEBUG
    #pragma message (PUB_L1_MISC_ENTER_STRING(PUB_L1_MISC_ENTER_CONCAT(BadLibVersionCheck___,PUB_L1_MISC_ENTER__FILENAME)))
    #endif
  /* This part goes into 
    every stack file using the ccdgen generated files (one for each used file)

    expands to e.g. 
      extern char BadLibVersionCheck____P_XX_TDC_3_VAL_H____Thu_Dec_12_17_49_56_2002
  */
  extern char PUB_L1_MISC_ENTER_SANITY_NAME;
  /* Originally it was this but since no one actually uses this stuff (we only have 
    it for the linker to check that versions match) we can save memory by only 
    storing the 8 lower bits.

    expands to e.g. 
      static char BadLibVersionCheck____P_XX_TDC_3_VAL_H = (char)(&BadLibVersionCheck____P_XX_TDC_3_VAL_H____Thu_Dec_12_17_49_56_2002);
  */
  static char PUB_L1_MISC_ENTER_CONCAT(BadLibVersionCheck___,PUB_L1_MISC_ENTER__FILENAME) = (char)(&PUB_L1_MISC_ENTER_SANITY_NAME);
  #endif
#endif
