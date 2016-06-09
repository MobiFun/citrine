/*
 * This module is our FreeCalypso adaptation of
 * g23m/condat/frame/config/gprsdrv.c from the Leonardo semi-src.
 * I renamed it from gprsdrv.c to gpf_tst_drv.c because nothing
 * in this module is specific to the GPRS configuration.
 */

#ifndef __GPRSDRV_C__
#define __GPRSDRV_C__
#endif

#include "gpfconf.h"

#ifndef _TARGET_
#define NEW_ENTITY
#endif

#ifdef _TARGET_
  #ifdef FF_TRACE_OVER_MTST
    #define MTST_TRACE
  #else
    #define TI_TRACE
  #endif
#endif

#include "gdi.h"
#include "vsi.h"
#include "pei.h"
/* #include "gprsconst.h" */
#include "frm_defs.h"
#include "frm_types.h"

/*==== TYPES ======================================================*/


/*==== CONSTANTS ==================================================*/

#if defined _TARGET_ && !defined PCON
 #define TR_RCV_BUF_SIZE    1024
#else
 #define TR_RCV_BUF_SIZE    1024
#endif

#define TR_MAX_IND         (TR_RCV_BUF_SIZE-1)

/*==== EXTERNALS ==================================================*/

#ifdef TI_TRACE
extern USHORT TIF_Init   ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc,
                           T_DRV_EXPORT const **DrvInfo );
extern USHORT TR_Init    ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc,
                           T_DRV_EXPORT const **DrvInfo );
extern USHORT TITRC_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc,
                           T_DRV_EXPORT const **DrvInfo );
#else
extern USHORT TIF_Init     ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc,
                             T_DRV_EXPORT const **DrvInfo );
extern USHORT TR_Init      ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc,
                             T_DRV_EXPORT const **DrvInfo );
extern USHORT SER_Init     ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc,
                             T_DRV_EXPORT const **DrvInfo );
#endif
extern USHORT mux_Init ( USHORT DrvHandle, T_DRV_CB_FUNC CallbackFunc,
                         T_DRV_EXPORT const **DrvInfo );
/*==== VARIABLES ==================================================*/

ULONG TR_RcvBufferSize = TR_RCV_BUF_SIZE;
ULONG TR_MaxInd = TR_MAX_IND;

const T_DRV_LIST_ENTRY DrvList[] =
{
  { NULL,   NULL,         NULL,  NULL },
#ifdef TI_TRACE
  { "TIF",  TIF_Init,     "RCV", NULL },
  { "TR",   TR_Init,      NULL,  NULL },
  { "TITRC",TITRC_Init,   NULL,  "" },
#else 
  #ifdef MTST_TRACE 
  { "TIF",  TIF_Init,     "RCV", NULL },
  { "TR",   TR_Init,      NULL,  NULL },
  { "MUX",  mux_Init,     NULL,  "" },
  #else
  { "TIF",  TIF_Init,     "RCV", NULL },
  { "TR",   TR_Init,      NULL,  NULL },
  { "SER",  SER_Init,     NULL,  ""   },
  #endif
#endif
  { NULL,   NULL,         NULL,  NULL }
};

int vsi_o_trace (char *caller, ULONG tclass, char *text,...)
{
  return 0;
}
