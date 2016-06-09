/* 
+------------------------------------------------------------------------------
|  File:       typedefs.h
+------------------------------------------------------------------------------
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
|  Purpose :  Standard definitions.
+----------------------------------------------------------------------------- 
*/ 

#ifndef __TYPEDEFS_H__
#define __TYPEDEFS_H__

/*===== Include operating system specific type definitions ========*/
#ifdef _VXWORKS_
  #include <vxWorks.h>
#endif

#ifdef PSOS
  #include <stdio.h>
#endif

/*==== CONSTANTS ==================================================*/

#ifndef __INCvxWorksh
#define IMPORT  EXTERN
#endif

#ifndef __cplusplus
  #define EXTERN  extern
#else
  #define EXTERN  extern "C"
#endif
#define LOCAL   static
#define GLOBAL  
#define EXPORT GLOBAL

/*lint -e723 supress Info -- Suspicious use of = */
#define EQ  ==
/*lint +e723 */
#define NEQ !=
#define AND &&
#define OR  ||
#define XOR(A,B) ((!(A) AND (B)) OR ((A) AND !(B)))

#ifndef FALSE
  #define FALSE 0
#endif

#ifndef TRUE
  #define TRUE 1
#endif

#ifndef NULL
  #define NULL 0
#endif

/*==== TYPES ======================================================*/
#ifndef STDDEFS_H
  typedef unsigned char       U8;
  typedef signed char         S8;
  typedef unsigned short      U16;
  typedef signed short        S16;
  typedef unsigned long       U32;
  typedef signed long         S32;
  typedef U32                 MEMHANDLE;
/*
 * UINT16 added for TI include files, to be removed ASAP 
 */ 

#ifndef GENERAL_H  /* rivera include definitions are as ours */
#ifndef __INCvxTypesOldh
  #if !defined NUCLEUS || !defined PLUS_VERSION_COMP
    /* UINT16 is already defined in the nucleus.h for the arm9 */
    typedef unsigned short UINT16;
  #endif
#endif


  typedef unsigned char       UBYTE;
  typedef short               SHORT;
  typedef UBYTE               BYTE;

  #if !defined (NUCLEUS)
  typedef char                CHAR;
  #endif

  /* the following construction assumes that we are on I86 using Windows. 
     It is introduced to avoid using WIN32 in GPF but keeping the compatibility with the 
     protocol stack 
  */
  #if defined WIN32 || defined _WIN32
    typedef int               BOOL;
  #else
    #ifdef _VXWORKS_
      #ifndef __INCvxTypesOldh
        typedef int               BOOL;
      #endif
    #else
      typedef UBYTE             BOOL;
    #endif
  #endif


  #ifndef _TYPES_H
   #ifndef __INCvxTypesOldh
      typedef unsigned char       UCHAR;
      typedef unsigned short      USHORT;
      typedef unsigned long       ULONG;
      typedef unsigned int        UINT;
    #endif
  #endif

#endif /* rivera include definitions are as ours */

  typedef long                LONG;
  
  typedef unsigned long T_VOID_STRUCT;

  typedef unsigned long T_ENUM; 

#endif


/*==== EXPORT =====================================================*/

#define MAXIMUM(A,B) (((A)>(B))?(A):(B))

#define MINIMUM(A,B) (((A)<(B))?(A):(B))

/*
 * NOTE: This is necessary until all occurences of Sprintf() in the
 * protocol stack (GSM and GPRS) have been replaced with sprintf().
 */
#define Sprintf sprintf


/*
 * NOTE: The following macros redefine the predefined macros of
 * the TMS470 compiler. This is usefull for creating binary
 * equivalent object files. These files can be used for the
 * comparison of to builds e.g. BUSYB and g23.pl.
 *
 * This approach may not work with other compilers.
 */
#define TMS470_CDS "__NO_DATE__"
#define TMS470_CTS "__NO_TIME__"
#define TMS470_CFS "__NO_FILE__"

#endif

