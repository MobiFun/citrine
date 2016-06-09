/* 
+------------------------------------------------------------------------------
|  File:       header.h
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
|  Purpose :  Common header types.
+----------------------------------------------------------------------------- 
*/ 

#include "glob_defs.h"

#ifndef __HEADER_H__
#define __HEADER_H__


/*
 * T_PRIM_HEADER
 *
 * Description :  This type definition defines the custom specific
 *                part of a primitive. All primitives have the
 *                general format: header followed by data. The
 *                header of a primitive is changeable according to
 *                the requirements of the target system.
 * Hints:         Only the operation code opc as a USHORT value must
 *                be present. For multi-instance protocol stacks
 *                the routing information must be include in the
 *                header (T_ROUTE route).
 */

#if !defined (T_SDU_DEFINED)

#define T_SDU_DEFINED

typedef struct
{
  USHORT l_buf;
  USHORT o_buf;
  UBYTE  buf[1];
} T_sdu;

#endif /* T_SDU_DEFINED */

/*
 * list of generic data descriptors
 */

#ifndef __T_desc_list__
#define __T_desc_list__

typedef struct
{
  U16             list_len;       /*<  0:  2> length in octets of whole data                     */
  U8              _align0;        /*<  2:  1> alignment                                          */
  U8              _align1;        /*<  3:  1> alignment                                          */
  U32             first;          /*<  4:  4> pointer to first generic data descriptor           */
} T_desc_list;
#endif

#ifndef __T_desc_list2__
#define __T_desc_list2__

typedef struct
{
  U16             list_len;       /*<  0:  2> length in octets of whole data                     */
  U8              _align0;        /*<  2:  1> alignment                                          */
  U8              _align1;        /*<  3:  1> alignment                                          */
  U32             first;          /*<  4:  4> pointer to first generic data descriptor           */
} T_desc_list2;
#endif

#ifndef __T_desc_list3__
#define __T_desc_list3__

typedef struct
{
  U16             list_len;       /*<  0:  2> length in octets of whole data                     */
  U8              _align0;        /*<  2:  1> alignment                                          */
  U8              _align1;        /*<  3:  1> alignment                                          */
  U32             first;          /*<  4:  4> pointer to first generic data descriptor           */
} T_desc_list3;
#endif

/*
 * generic data descriptor
 */
#ifndef __T_desc__
#define __T_desc__                /* to prevent double include in generated files */

typedef struct
{
  ULONG           next;           /* next generic data descriptor   */
  USHORT          len;            /* length of content in octets    */
  UBYTE           buffer[1];      /* buffer content                 */
} T_desc;
#endif /* __T_desc__ */

#ifndef __T_desc2__
#define __T_desc2__                /* to prevent double include in generated files */

typedef struct
{
  U32             next;            /*<  0:  4> next generic data descriptor        */
  U16             offset;          /*<  4:  2> offset in octets                    */
  U16             len;             /*<  6:  2> length of content in octets         */
  U16             size;            /*<  8:  2> size of buffer in octets            */
  U8              buffer[1];       /*< 10:  1> buffer content                      */
} T_desc2;
#endif /* __T_desc2__ */

#ifndef __T_desc3__
#define __T_desc3__

typedef struct
{
  U32             next;            /*<  0:  4> next generic data descriptor        */
  U16             offset;          /*<  4:  2> offset in octets                    */
  U16             len;             /*<  6:  2> length of content in octets         */
  U32             buffer;          /*<  8:  4> pointer to buffer                   */
} T_desc3;
#endif

#if !defined (T_FRAME_DESC_DEFINED)

#define T_FRAME_DESC_DEFINED

typedef struct
{
  UBYTE  *Adr[2];
  USHORT  Len[2];
} T_FRAME_DESC;

#endif /* T_FRAME_DESC_DEFINED */

#ifdef OPTION_MULTI_INSTANCES
typedef struct
{
  USHORT  inst_no;
  USHORT  chan_no;
  UBYTE   ts_no;
} T_ROUTE;
#endif

typedef struct
{
  char entity;
  char dir;
  char type;
  char align1;
} T_SDU_TRACE;

#if !defined (T_PRIM_HEADER_DEFINED)
#define T_PRIM_HEADER_DEFINED

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   In the current implementation it is essential that the
   T_PRIM_HEADER type has the same size as the T_DP_HEADER
   type and that the element use_cnt is at the same position
   in both header types!
   ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
typedef struct
{
  ULONG opc;         /* equal to int SignalCode            */
  ULONG len;         /* primitive length                   */
  LONG  use_cnt;     /* counter indicates cuurent number of users */
  T_sdu *sdu;        /* pointer to sdu struct if available */
  ULONG sh_offset;   /* offset of system header            */
  ULONG dph_offset;  /* offset of dynamic prim header      */
} T_PRIM_HEADER;

#endif /* T_PRIM_HEADER_DEFINED */

typedef struct _T_DP_HEADER
{
  ULONG magic_nr;       /* magic number is checked at each access */
  ULONG size;           /* available bytes in dynamic primitive partition */
  ULONG use_cnt;        /* counter indicates current number of users */
  ULONG offset;         /* offset from partition begin to next free byte */
  T_VOID_STRUCT**
        drp_bound_list; /* pointer to the list of dynamic partitions bound to this partition */  
  struct _T_DP_HEADER *next;
} T_DP_HEADER;

typedef struct
{
  SHORT ref_cnt;     /* ref_cnt for MALLOC partitions */
  SHORT desc_type;   /* descriptor type */
} T_M_HEADER;

/*
 * flags in the opc
 */
#define EXTENDED_OPC          0x80000000
#define VIRTUAL_OPC           0x40000000
#define DOWNLINK_OPC          0x00004000
#define UPLINK_OPC            0x00000000
#define MEMHANDLE_OPC         0x20000000

#define SAP32_MASK      0x00007fff   /* UL/DL Bit Part of SAP Nr.*/
#define SAP16_MASK      0x7f00
#define PRIM32_MASK     0x00ff0000
#define PRIM16_MASK     0x00ff
#define OPC32BIT(opc)   (opc&EXTENDED_OPC)
/* 
 * for 16bit opcs SAP_NR() returns the same result as (opc & SAP_MASK) in the old style. 
 * No shift right is done, e.g. 0x4d00 is returned instead of 0x4d.
 * Also the UL/DL bit is part of the SAP to be downwards compatible with the
 * existing code in the xxx_pei.c modules.
 */
#define SAP_NR(opc)     (USHORT)((opc&0x80000000)?(opc&SAP32_MASK):(opc&SAP16_MASK))
#define PRIM_NR(opc)    (USHORT)((opc&0x80000000)?((opc&PRIM32_MASK)>>16):(opc&PRIM16_MASK))

#define HANDLE_BIT           ((UBYTE)0x80)
#define HANDLE_MASK          ((UBYTE)0x7F)

typedef struct
{
  ULONG magic_nr;
  ULONG time;
  char  snd     [RESOURCE_NAMELEN];
  char  rcv     [RESOURCE_NAMELEN];
  char  org_rcv [RESOURCE_NAMELEN];
} T_S_HEADER;

typedef struct
{
  T_PRIM_HEADER p_hdr;
  T_PRIM_HEADER *prim_ptr;
} T_PRIM_X;

#endif /* HEADER_H */

