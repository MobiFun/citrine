/* 
+------------------------------------------------------------------------------
|  File:       stddefs.h
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
|  Purpose :  Old frame standard definitions.
+----------------------------------------------------------------------------- 
*/ 


#ifndef __STDDEFS_H__
#define __STDDEFS_H__

#ifndef OLD_FRAME
 #ifndef NEW_FRAME
  #define NEW_FRAME
 #endif
#endif

/*==== CONSTANTS ==================================================*/

#define FALSE  0
#define TRUE   1

#define BIT123  0x07
#define BIT4    0x08
#define BIT78   0xC0

#if defined (USE_DLL)
  #ifdef WIN32
    #define EXPORT __declspec (dllexport)
    #define IMPORT __declspec (dllimport)
  #endif
#else
  #define EXPORT GLOBAL
  #define IMPORT EXTERN
#endif

#ifndef __cplusplus
  #define EXTERN extern 
#else 
  #define EXTERN extern "C"
#endif

/* Horrible handling of GLOBAL and MAINFILE macros
   Documented in BugTrak#DNCL-3PWCL3 */
#ifdef L1_LIB
  #ifdef L1_COM_C
    #define GLOBAL 
    #define MAINFILE 1
  #else
    #ifndef __cplusplus   
      #define GLOBAL extern 
    #else 
      #define GLOBAL extern "C"
    #endif
  #endif
#else
  #define GLOBAL
#endif

#define LOCAL   static

#define AND &&
#define OR  ||
#define XOR(A,B) ((!(A) AND (B)) OR ((A) AND !(B)))

#define EQ  ==
#define NEQ !=

#ifndef NULL
#define NULL    0
#endif
/*==== TYPES ======================================================*/
#ifndef WIN32
  typedef char           BYTE;
#else
  typedef unsigned char  BYTE;
#endif

#if !defined (NUCLEUS)
typedef char           CHAR;
#endif



typedef long           LONG;
typedef unsigned char  UBYTE;
typedef unsigned char  UCHAR;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;
typedef unsigned short UINT16;
typedef unsigned long  UINT32;
typedef unsigned short UNSIGNED16;

typedef unsigned long T_VOID_STRUCT;

#ifndef L1_LIB
typedef short          SHORT;
#ifdef WIN32
  typedef int            BOOL;
#else
  typedef UBYTE          BOOL;
#endif
#endif

#if !defined (T_SDU_DEFINED)

#define T_SDU_DEFINED

typedef struct
{
  USHORT l_buf;
  USHORT o_buf;
  UBYTE  buf[1];
} T_sdu;

#endif /* T_SDU_DEFINED */


#if !defined (T_FRAME_DESC_DEFINED)

#define T_FRAME_DESC_DEFINED

typedef struct 
{
  UBYTE  *Adr[2];
  USHORT  Len[2];
} T_FRAME_DESC;

#endif /* T_FRAME_DESC_DEFINED */


/*
 * for definitions of jumptables
 */
#ifndef NEW_ENTITY
 typedef void   (*T_VOID_FUNC)();
 typedef short  (*T_SHORT_FUNC)();
#endif
typedef USHORT (*T_USHORT_FUNC)();

/* SDU_TRAIL denotes the memory size starting at 'buf' to the end of T_sdu */

#define  SDU_TRAIL  ((char*)(((T_sdu*)0)+1)-(char*)(((T_sdu*)0)->buf))

/*==== EXPORT =====================================================*/
#define MAXIMUM(A,B) (((A)>(B))?(A):(B))

#define MINIMUM(A,B) (((A)<(B))?(A):(B))

#define _STR(x)  __STR(x)
#define __STR(x) #x

#define ALERT(CONDITION,TEXT) \
        {														   \
		   if (!(CONDITION))									   \
		      ext_abort (#TEXT "\n" __FILE__ ", line " _STR(__LINE__)); \
		}


/*==== MISC =======================================================*/


#endif

