/* 
+------------------------------------------------------------------------------
|  File:       tstheader.h
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
|  Purpose :  Definitions for the TST internal header 
+----------------------------------------------------------------------------- 
*/ 

#ifndef TSTHEADER_H
#define TSTHEADER_H

/*==== INCLUDES =============================================================*/

#include "gpfconf.h"
#include "tools.h" 

/*==== CONSTS ===============================================================*/

#ifdef CONNECT_2_PS
#define TOOLSIDE
#else
#define STACKSIDE
#endif

#if defined STACKSIDE && defined _TARGET_
#define TARGET_STACK
#endif

#if defined STACKSIDE && !defined _TARGET_
#define EMULATED_STACK
#endif


#define ID_OFFSET             0
#define TIMESTAMP_OFFSET      1
#define LENGTH_OFFSET         5
#define SENDER_OFFSET         9
#define RECEIVER_OFFSET      13
#define DATA_OFFSET          17

#define ID_SUBTRACT          17
#define TIMESTAMP_SUBTRACT   16
#define LENGTH_SUBTRACT      12
#define SENDER_SUBTRACT       8
#define RECEIVER_SUBTRACT     4
#define OPC_SUBTRACT          4
#define DATA_SUBTRACT         0

#define PROT_PRIM_ID        'P'
#define PROT_PRIM_ID_32BIT  'Q'
#define SYS_PRIM_ID         'S'
#define TRACE_ID            'T'

#define IDENT_PS_PRIM       0x10  /* former 'P' */
#define IDENT_SYS_PRIM      0x30  /* former 'S' */
#define IDENT_ABORT         0x00  /* former 'A' */
#define IDENT_TRACE         0x20  /* former 'T' */


#define HDR_VALID_VERSION_0   0x40  /* 01 OLD TST Header */
#define HDR_VALID_VERSION_1   0x80  /* 10 NEW TST Header */
#define HDR_VALID_VERSION_2   0x00  /* 00 reserved       */
#define HDR_VALID_VERSION_3   0xC0  /* 11 reserved       */

#define HDR_TIME_MS     0x04  /* 01 ms Time Frame   */
#define HDR_TIME_TDMA   0x08  /* 10 TDMA Time Frame */

#define HDR_VERSION_MASK      0xc0  
#define HDR_IDENT_MASK        0x30  
#define HDR_TIME_MASK         0x0c  
#define HDR_RESERVED_MASK     0x03  

#define LOW_MASK 0xFF

#define EMPTY_BYTE  0
#define INFO_BYTE   1
#define FIRST_BYTE  2
#define SECOND_BYTE 3

#define TST_HEADER_LEADING_FIELDS 3   /* .info + .size */
#define TST_HEADER_TRAILING_FIELDS 12 /* .time + .sender + .receiver */
/*  the .orgreceiver field will be added dynamically, if used */

/*==== TYPES =================================================================*/
typedef struct
{
  UBYTE combined [4];
  unsigned long time;
  char sender[4];
  char receiver[4];
} TST_SMALL_HEADER;

typedef struct
{
  UBYTE combined [4];
  unsigned long time;
  char sender[4];
  char receiver[4];
  UCHAR trace_opc;
} TST_MED_HEADER;

typedef struct
{
  UBYTE combined [4];
  unsigned long time;
  char sender[4];
  char receiver[4];
  char orgreceiver[4];
  int opc;
} TST_BIG_HEADER;

#define TST_SMALL_HEADER_SIZE (sizeof(TST_SMALL_HEADER) -1)
#define TST_BIG_HEADER_SIZE (sizeof(TST_BIG_HEADER) -1)

#if defined (_LINUX_) || defined (_SOLARIS_)
#define PRIM_HEADER_FLAG 0x00000000
#define PRIM_DATA_FLAG   0x00000000
#define PRIM_FLAG_MASK   0x00000000

#define EXCHANGE_4BYTES_ENDIANESS(val_ptr)    {\
                                                char c;\
                                                char *p = (char*) val_ptr;\
                                                c = p[0];\
                                                p[0] = p[3];\
                                                p[3] = c;\
                                                c = p[1];\
                                                p[1] = p[2];\
                                                p[2] = c;\
                                              }
#else
#define PRIM_HEADER_FLAG 0x40000000
#define PRIM_DATA_FLAG   0x80000000
#define PRIM_FLAG_MASK   0xc0000000

#define EXCHANGE_4BYTES_ENDIANESS(val_ptr)
#endif

/*==== EXPORTS ===============================================================*/

#endif /* !TSTHEADER.H */
