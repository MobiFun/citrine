/* 
+----------------------------------------------------------------------------- 
|  Project :  
|  Modul   :  pdi.h
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
|  Purpose :  
+----------------------------------------------------------------------------- 
*/ 

#ifndef __PDI_H__
#define __PDI_H__

/*==== INCLUDES =============================================================*/
#include "typedefs.h"
#include "ccdapi.h"

/*==== CONSTANTS =============================================================*/
#define PD_XX                     1
#define PD_CC                     3
#define PD_MM                     5
#define PD_RR                     6
#define PD_GMM                    8
#define PD_SMS                    9
#define PD_SS                     11
#define PD_SM                     10
#define PD_TST                    15

#define PDI_MAXDECODEINFOATTRIB  128
#define PDI_MAXDECODEINFOPRIM    128
#define PDI_MAXDECODEINFOENTITY  128
#define PDI_MAXPMEMFORMTYPE       23

#define PDI_DECODETYPE_L3PDU              0
#define PDI_DECODETYPE_L3PDU_N            1
#define PDI_DECODETYPE_SAPI               2
#define PDI_DECODETYPE_NOPD               3
#define PDI_DECODETYPE_NOPD_NOTYPE        4
#define PDI_DECODETYPE_NOPD_N             5
#define PDI_DECODETYPE_NOPD_NOTYPE_N      6
#define PDI_DECODETYPE_RR_SHORT           7
#define PDI_DECODETYPE_MAC_H              8
#define PDI_DECODETYPE_MAC_H_N            9
#define PDI_DECODETYPE_MAC_H_CHECK       10
#define PDI_DECODETYPE_MAC_H_N_CHECK     11
#define PDI_DECODETYPE_AIM               12
#define PDI_DECODETYPE_AIM_N             13
#define PDI_DECODETYPE_AIM_CHECK         14
#define PDI_DECODETYPE_AIM_N_CHECK       15

#define PDI_DLL_ERROR             -2

/* returned decoding info */
typedef struct
{
  UBYTE         entity;
  UBYTE         dir;
  unsigned char pd;
  unsigned char ti;
  T_MSGBUF      *mbuf;
  UBYTE         msg_type;
} T_PDI_CCDMSG;

typedef int (*T_pdi_prepare_ccdmsg)(T_PDI_CCDMSG* ccdmsg, ULONG values[], int len);

typedef struct
{
  char  type;
  char  attrib[PDI_MAXDECODEINFOATTRIB];
  char  prim[PDI_MAXDECODEINFOPRIM];
  char  entity[PDI_MAXDECODEINFOENTITY];
  UBYTE msg_type;
  T_pdi_prepare_ccdmsg pdi_prepare_ccdmsg;
  char** primmbr;
} T_PDI_DECODEINFO;


/*
 *   internal context data
 */
typedef short T_PDI_PdEntityTable[16];
typedef struct
{
  T_PDI_PdEntityTable PdEntityTable;
  T_PDI_DECODEINFO***  PrimDecodeInfo;

  T_PDI_DECODEINFO *dinfo;
  USHORT sap;
  USHORT opc;
  UBYTE  dir;
  USHORT pmtx;

  unsigned char sapi;
  ULONG mtypeval[PDI_MAXPMEMFORMTYPE];
  int   mtypenum;
  UBYTE* mi_length;
} T_PDI_CONTEXT;

typedef struct
{
  enum {PDI_NONE, PDI_CCDMSG} decodetype;
  union _pdi
  {
    T_PDI_CCDMSG  ccdmsg;
  } pdi;
} T_PDI;


#if !defined (CCDDATA_PREF)
#if defined (_WIN32_) && defined (CCDDATA_LOAD)
#define CCDDATA_PREF(pdi_fun) cddl_##pdi_fun
#else
#define CCDDATA_PREF(pdi_fun) pdi_fun
#endif /* _WIN32_ && CCDDATA_LOAD */
#endif /* !CCDDATA_PREF */

/*
 *   create new default context
 */
T_PDI_CONTEXT* CCDDATA_PREF(pdi_createDefContext)();

/*
 *   create new context
 */
T_PDI_CONTEXT* CCDDATA_PREF(pdi_createContext)(const T_PDI_DECODEINFO *dinfop, unsigned int dicount);

/*
 *   destroy context
 */
void CCDDATA_PREF(pdi_destroyContext)(T_PDI_CONTEXT *context);

/*
 *   mark the begin of a new primitive
 */
void CCDDATA_PREF(pdi_startPrim)(T_PDI_CONTEXT *context, ULONG opc);

/*
 *   returns extended decode information for a given
 *   ccdedit element descriptor
 */
void CCDDATA_PREF(pdi_getDecodeInfo)(T_PDI_CONTEXT *context, const char *ename,
                       char *evalue, int evlen, T_PDI *decinfo);

short CCDDATA_PREF(pdi_getEntityByPD)(const T_PDI_CONTEXT *context, unsigned char pd);

const char* CCDDATA_PREF(pdi_pd2name)(unsigned char pd);

#endif // __PDI_H__
